#include "QtForge.h"
#include <QDebug>
#include <QCoreApplication>
#include <QTimer>
#include <QtQml/qqml.h>
#include <QQmlPropertyMap>
#include <QUuid>

QtForge::QtForge(QObject *parent) noexcept
    : QObject(parent)
    , m_engine(nullptr)
{
}

void QtForge::setupEngine(QQmlApplicationEngine *engine) noexcept
{
    if (!engine) [[unlikely]] {
        qWarning() << "QtForge: Cannot setup engine with null pointer";
        return;
    }
    
    m_engine = engine;
    
    // Подключаемся к сигналу objectCreated для обработки ошибок и привязок
    connect(engine, &QQmlApplicationEngine::objectCreated,
            this, &QtForge::onObjectCreated, Qt::DirectConnection);
    
    qDebug() << "QtForge: Engine setup completed";
}

void QtForge::registerBinding(const QString &objectId, const QString &propertyName, QObject *value)
{
    if (!value) {
        qWarning() << "QtForge: Cannot register binding with null value";
        return;
    }
    
    Binding binding;
    binding.objectId = objectId;
    binding.propertyName = propertyName;
    binding.value = value;
    
    m_bindings.append(binding);
    qDebug() << "QtForge: Registered binding - objectId:" << objectId 
             << "propertyName:" << propertyName;
}



bool QtForge::load(const QUrl &url) noexcept
{
    if (!m_engine) [[unlikely]] {
        qWarning() << "QtForge: Cannot load - engine not set up";
        return false;
    }
    
    m_mainUrl = url;
    
    qDebug() << "QtForge: Loading QML file:" << url;
    m_engine->load(url);
    return true;
}

void QtForge::onObjectCreated(QObject *obj, const QUrl &objUrl)
{
    // Проверяем, что это наш главный объект
    if (objUrl != m_mainUrl) {
        return;
    }
    
    // Обработка ошибок загрузки
    if (!obj) {
        qCritical() << "QtForge: Failed to load QML file:" << m_mainUrl;
        QCoreApplication::exit(-1);
        return;
    }
    
    qDebug() << "QtForge: Main object created, applying bindings and contexts...";
    
    // Используем QTimer для установки привязок после полной инициализации QML
    // Это гарантирует, что объекты будут найдены и контексты будут готовы
    QTimer::singleShot(0, this, [this, obj]() {
        // Применяем все зарегистрированные привязки
        for (const auto &binding : m_bindings) {
            if (!binding.value) [[unlikely]] {
                qWarning() << "QtForge: Binding value is null for" << binding.objectId;
                continue;
            }
            
            // Ищем объект по objectName (не по id!)
            auto *targetObject = obj->findChild<QObject*>(binding.objectId);
            
            if (!targetObject) [[unlikely]] {
                qWarning() << "QtForge: Object with objectName" << binding.objectId << "not found";
                continue;
            }
            
            // Устанавливаем свойство напрямую в объект компонента через setProperty
            // Это обеспечивает изоляцию - каждое свойство доступно только в своем компоненте
            const auto propertyNameBytes = binding.propertyName.toUtf8();
            const bool success = targetObject->setProperty(propertyNameBytes.constData(), 
                                                          QVariant::fromValue(binding.value));
            
            if (success) [[likely]] {
                qDebug() << "QtForge: Applied binding - objectName:" << binding.objectId 
                         << "propertyName:" << binding.propertyName;
            } else {
                qWarning() << "QtForge: Failed to set property" << binding.propertyName 
                          << "on object" << binding.objectId;
            }
        }
        
        // Автоматически применяем локальные контексты к объектам
        for (auto it = m_objectContextMapping.constBegin(); it != m_objectContextMapping.constEnd(); ++it) {
            const QString &objectName = it.key();
            const QString &contextId = it.value();
            (void)applyContextToObject(objectName, contextId);
        }
    });
}

bool QtForge::setContextProperty(const QString &name, QObject *value, const QString &contextId) noexcept
{
    if (!value) [[unlikely]] {
        qWarning() << "QtForge: Cannot set context property with null value";
        return false;
    }
    
    if (!m_engine) [[unlikely]] {
        qWarning() << "QtForge: Cannot set context property - engine not initialized";
        return false;
    }
    
    QQmlContext *targetContext = nullptr;
    
    // Если contextId пустая, используем root context
    if (contextId.isEmpty()) {
        targetContext = m_engine->rootContext();
        if (!targetContext) [[unlikely]] {
            qWarning() << "QtForge: Cannot set context property - root context is null";
            return false;
        }
    } else {
        // Ищем локальный контекст по ID
        auto it = m_localContexts.find(contextId);
        if (it == m_localContexts.end()) [[unlikely]] {
            qWarning() << "QtForge: Cannot set context property - context ID not found:" << contextId;
            return false;
        }
        targetContext = it.value();
    }
    
    targetContext->setContextProperty(name, value);
    qDebug() << "QtForge: Set context property - name:" << name 
             << (contextId.isEmpty() ? " (root context)" : " (local context:" + contextId + ")");
    return true;
}

bool QtForge::registerSingletonInstance(const QString &module, const QString &name, QObject *instance) noexcept
{
    if (!instance) [[unlikely]] {
        qWarning() << "QtForge: Cannot register singleton instance - instance is null";
        return false;
    }
    
    if (!m_engine) [[unlikely]] {
        qWarning() << "QtForge: Cannot register singleton instance - engine not set up";
        return false;
    }
    
    // Используем qmlRegisterSingletonInstance для регистрации готового экземпляра
    const QByteArray moduleBytes = module.toUtf8();
    const QByteArray nameBytes = name.toUtf8();
    
    qmlRegisterSingletonInstance(moduleBytes.constData(), 1, 0, nameBytes.constData(), instance);
    qDebug() << "QtForge: Registered singleton instance - module:" << module << "name:" << name;
    return true;
}

QQmlPropertyMap* QtForge::registerPropertyMap(const QString &name, QObject *parent) noexcept
{
    if (!m_engine) [[unlikely]] {
        qWarning() << "QtForge: Cannot register property map - engine not set up";
        return nullptr;
    }
    
    auto *propertyMap = new QQmlPropertyMap(parent ? parent : this);
    
    auto *rootContext = m_engine->rootContext();
    if (rootContext) [[likely]] {
        rootContext->setContextProperty(name, propertyMap);
        qDebug() << "QtForge: Registered property map - name:" << name;
        return propertyMap;
    }
    
    delete propertyMap;
    return nullptr;
}

QString QtForge::createLocalContext(QObject *parent) noexcept
{
    if (!m_engine) [[unlikely]] {
        qWarning() << "QtForge: Cannot create local context - engine not initialized";
        return QString();
    }
    
    auto *rootContext = m_engine->rootContext();
    if (!rootContext) [[unlikely]] {
        qWarning() << "QtForge: Cannot create local context - root context is null";
        return QString();
    }
    
    // Создаем локальный контекст
    auto *context = new QQmlContext(rootContext, parent ? parent : this);
    
    // Генерируем уникальный ID
    QString contextId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    
    // Сохраняем в хранилище
    m_localContexts[contextId] = context;
    
    qDebug() << "QtForge: Created local context - ID:" << contextId;
    return contextId;
}

bool QtForge::applyContextToObject(const QString &objectName, const QString &contextId) noexcept
{
    if (!m_engine) [[unlikely]] {
        qWarning() << "QtForge: Cannot apply context - engine not initialized";
        return false;
    }
    
    // Находим контекст по ID
    auto it = m_localContexts.find(contextId);
    if (it == m_localContexts.end()) [[unlikely]] {
        qWarning() << "QtForge: Cannot apply context - context ID not found:" << contextId;
        return false;
    }
    
    QQmlContext *context = it.value();
    
    // Находим root объект
    const auto rootObjects = m_engine->rootObjects();
    if (rootObjects.isEmpty()) [[unlikely]] {
        qWarning() << "QtForge: Cannot apply context - no root objects";
        return false;
    }
    
    QObject *rootObject = rootObjects.first();
    
    // Ищем объект по objectName
    QObject *targetObject = rootObject->findChild<QObject*>(objectName);
    if (!targetObject) [[unlikely]] {
        qWarning() << "QtForge: Cannot apply context - object not found:" << objectName;
        return false;
    }
    
    // Применяем контекст к объекту
    QQmlEngine::setContextForObject(targetObject, context);
    qDebug() << "QtForge: Applied context - objectName:" << objectName << "contextId:" << contextId;
    return true;
}

QString QtForge::createComponent(const QUrl &url, QObject *parent) noexcept
{
    if (!m_engine) [[unlikely]] {
        qWarning() << "QtForge: Cannot create component - engine not initialized";
        return QString();
    }
    
    // Создаем компонент
    auto *component = new QQmlComponent(m_engine, url, parent ? parent : this);
    
    // Генерируем уникальный ID
    QString componentId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    
    // Сохраняем в хранилище
    m_components[componentId] = component;
    
    qDebug() << "QtForge: Created component - ID:" << componentId << "URL:" << url;
    return componentId;
}

bool QtForge::registerComponentProperty(const QString &componentId, const QString &propertyName, QObject *value) noexcept
{
    if (!m_engine) [[unlikely]] {
        qWarning() << "QtForge: Cannot register component property - engine not initialized";
        return false;
    }
    
    if (!value) [[unlikely]] {
        qWarning() << "QtForge: Cannot register component property with null value";
        return false;
    }
    
    // Находим компонент по ID
    auto it = m_components.find(componentId);
    if (it == m_components.end()) [[unlikely]] {
        qWarning() << "QtForge: Cannot register component property - component ID not found:" << componentId;
        return false;
    }
    
    QQmlComponent *component = it.value();
    
    // Проверяем статус компонента
    if (component->status() != QQmlComponent::Ready) [[unlikely]] {
        qWarning() << "QtForge: Cannot register component property - component is not ready, status:" << component->status();
        return false;
    }
    
    // Создаем объект из компонента
    QObject *object = component->create();
    if (!object) [[unlikely]] {
        qWarning() << "QtForge: Failed to create object from component";
        return false;
    }
    
    // Устанавливаем свойство
    const auto propertyNameBytes = propertyName.toUtf8();
    const bool success = object->setProperty(propertyNameBytes.constData(), QVariant::fromValue(value));
    
    if (success) [[likely]] {
        // Сохраняем созданный объект
        QString objectId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        m_createdObjects[objectId] = object;
        qDebug() << "QtForge: Registered component property - componentId:" << componentId 
                 << "propertyName:" << propertyName << "objectId:" << objectId;
        return true;
    } else {
        qWarning() << "QtForge: Failed to set property" << propertyName << "on component object";
        delete object;
        return false;
    }
}

QObject* QtForge::createAndAddComponent(const QString &componentId, 
                                        const QString &propertyName, 
                                        QObject *value, 
                                        QObject *parentContainer) noexcept
{
    if (!m_engine) [[unlikely]] {
        qWarning() << "QtForge: Cannot create and add component - engine not initialized";
        return nullptr;
    }
    
    if (!value) [[unlikely]] {
        qWarning() << "QtForge: Cannot create and add component with null value";
        return nullptr;
    }
    
    if (!parentContainer) [[unlikely]] {
        qWarning() << "QtForge: Cannot create and add component - parent container is null";
        return nullptr;
    }
    
    // Находим компонент по ID
    auto it = m_components.find(componentId);
    if (it == m_components.end()) [[unlikely]] {
        qWarning() << "QtForge: Cannot create and add component - component ID not found:" << componentId;
        return nullptr;
    }
    
    QQmlComponent *component = it.value();
    
    // Проверяем статус компонента
    if (component->status() != QQmlComponent::Ready) [[unlikely]] {
        qWarning() << "QtForge: Cannot create and add component - component is not ready, status:" << component->status();
        return nullptr;
    }
    
    // Получаем QML контекст родительского контейнера
    QQmlContext *context = qmlContext(parentContainer);
    if (!context) [[unlikely]] {
        qWarning() << "QtForge: Cannot create and add component - cannot get QML context for parent container";
        return nullptr;
    }
    
    // Создаем объект из компонента с правильным контекстом и родителем
    QObject *object = component->create(context, parentContainer);
    if (!object) [[unlikely]] {
        qWarning() << "QtForge: Failed to create object from component";
        return nullptr;
    }
    
    // Устанавливаем свойство
    const auto propertyNameBytes = propertyName.toUtf8();
    const bool success = object->setProperty(propertyNameBytes.constData(), QVariant::fromValue(value));
    
    if (success) [[likely]] {
        // Сохраняем созданный объект для отслеживания (опционально)
        QString objectId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        m_createdObjects[objectId] = object;
        qDebug() << "QtForge: Created and added component - componentId:" << componentId 
                 << "propertyName:" << propertyName << "objectId:" << objectId;
        return object;
    } else {
        qWarning() << "QtForge: Failed to set property" << propertyName << "on component object";
        delete object;
        return nullptr;
    }
}


