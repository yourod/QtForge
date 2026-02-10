#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QUrl>
#include <QMap>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlPropertyMap>
#include <QAbstractListModel>

class QtForge : public QObject
{
    Q_OBJECT

public:
    explicit QtForge(QObject *parent = nullptr) noexcept;
    void setupEngine(QQmlApplicationEngine *engine) noexcept;
    [[nodiscard]] bool load(const QUrl &url) noexcept;
    void registerBinding(const QString &objectId, const QString &propertyName, QObject *value);
    [[nodiscard]] bool setContextProperty(const QString &name, QObject *value, const QString &contextId = QString()) noexcept;
    [[nodiscard]] QString createLocalContext(QObject *parent = nullptr) noexcept;
    [[nodiscard]] bool applyContextToObject(const QString &objectName, const QString &contextId) noexcept;
    [[nodiscard]] QString createComponent(const QUrl &url, QObject *parent = nullptr) noexcept;
    [[nodiscard]] bool registerSingletonInstance(const QString &module, const QString &name, QObject *instance) noexcept;
    [[nodiscard]] QQmlPropertyMap* registerPropertyMap(const QString &name, QObject *parent = nullptr) noexcept;
    [[nodiscard]] bool registerComponentProperty(const QString &componentId, const QString &propertyName, QObject *value) noexcept;
    
    // Создает компонент и автоматически добавляет его в указанный QML контейнер
    // Рекомендуемый способ работы с компонентами - компонент будет виден в интерфейсе
    [[nodiscard]] QObject* createAndAddComponent(const QString &componentId, 
                                                   const QString &propertyName, 
                                                   QObject *value, 
                                                   QObject *parentContainer) noexcept;
    

private slots:
    void onObjectCreated(QObject *obj, const QUrl &objUrl);

private:
    struct Binding {
        QString objectId;
        QString propertyName;
        QObject *value;
    };
    
    QList<Binding> m_bindings;
    QQmlApplicationEngine *m_engine;
    QUrl m_mainUrl;
    
    QMap<QString, QQmlContext*> m_localContexts;      // ID -> локальный контекст
    QMap<QString, QQmlComponent*> m_components;       // ID -> компонент
    QMap<QString, QObject*> m_createdObjects;        // ID -> созданный объект
    QMap<QString, QString> m_objectContextMapping;   // objectName -> contextId для автоматического применения
};

