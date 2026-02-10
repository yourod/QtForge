# QtForge

Обертка над `QQmlApplicationEngine`, которая упрощает работу с QML в Qt6. Я создал эту библиотеку, потому что постоянно сталкивался с одними и теми же задачами при интеграции C++ с QML — настройка контекстов, привязка объектов, управление компонентами. Вместо того чтобы каждый раз писать один и тот же код, решил собрать все в одну библиотеку.

## Что это дает

QtForge берет на себя рутину работы с QML движком. Вместо того чтобы вручную настраивать контексты и привязки, вы просто вызываете несколько методов. Библиотека автоматически применяет привязки после загрузки QML, управляет контекстами и упрощает регистрацию singleton'ов.

## Основные возможности

- **Настройка движка** — одна строка вместо нескольких вызовов
- **Загрузка QML** — с автоматической обработкой ошибок
- **Привязки объектов** — связывание C++ объектов с QML по `objectName`
- **Работа с контекстами** — root context для глобальных данных, локальные для изоляции
- **Singleton'ы** — регистрация готовых C++ объектов как QML singleton'ов
- **PropertyMap** — динамические свойства для случаев, когда структура данных меняется
- **Компоненты** — программное создание и настройка QML компонентов

## Требования и установка

Нужен Qt6 (Core и Quick), CMake 3.16+ и компилятор с поддержкой C++17. Проверено на Qt 6.2+, но должно работать и на более ранних версиях.

### Сборка

```bash
git clone <repository-url>
cd QtForge
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
sudo cmake --install .  # или без sudo для установки в ~/.local
```

После установки библиотека будет в стандартных местах:
- Заголовки: `/usr/local/include/QtForge.h` (или `~/.local/include/`)
- Библиотека: `/usr/local/lib/libQtForgeLib.a`
- CMake config: `/usr/local/lib/cmake/QtForge/`

**Примечание:** Если CMake не находит Qt6 автоматически (что часто бывает, если Qt установлен нестандартно), библиотека попытается найти его в `~/Qt`, `/usr/local/Qt` и других стандартных местах. Если не находит — установите переменную `Qt6_DIR` перед запуском cmake.

## Начало работы

Вот минимальный пример, чтобы понять, как это работает:

```cpp
#include <QtForge.h>
#include <QQmlApplicationEngine>
#include <QGuiApplication>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    QQmlApplicationEngine engine;
    QtForge forge;
    
    // Настройка движка
    forge.setupEngine(&engine);
    
    // Загрузка QML файла
    forge.load(QUrl("qrc:/main.qml"));
    
    return app.exec();
}
```

## Примеры использования

### Пример 1: Базовое использование

Самый простой случай — просто загрузить QML файл. Обратите внимание на порядок вызовов: сначала `setupEngine()`, потом `load()`. Если вызвать `load()` до `setupEngine()`, получите предупреждение и ничего не загрузится.

**main.cpp:**
```cpp
#include <QtForge.h>
#include <QQmlApplicationEngine>
#include <QGuiApplication>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    QQmlApplicationEngine engine;
    QtForge forge;
    
    // Важно: setupEngine должен быть вызван первым
    forge.setupEngine(&engine);
    
    // Загружаем QML. Можно использовать qrc:/ для ресурсов или file:// для файлов
    if (!forge.load(QUrl("qrc:/main.qml"))) {
        qCritical() << "Failed to load QML file";
        return -1;
    }
    
    return app.exec();
}
```

**main.qml:**
```qml
import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    width: 640
    height: 480
    visible: true
    title: "QtForge Example"
    
    Text {
        anchors.centerIn: parent
        text: "Hello, QtForge!"
        font.pixelSize: 24
    }
}
```

### Пример 2: Регистрация свойств в root context

Root context — это самый простой способ передать данные в QML. Все, что вы туда положите, будет доступно из любого QML компонента. Удобно для глобальных настроек, но не используйте для данных, которые должны быть изолированы (для этого есть локальные контексты).

**main.cpp:**
```cpp
#include <QtForge.h>
#include <QQmlApplicationEngine>
#include <QGuiApplication>
#include <QObject>

// Обычный QObject с свойствами — ничего особенного
class AppSettings : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString appName READ appName CONSTANT)
    Q_PROPERTY(int version READ version CONSTANT)
    
public:
    explicit AppSettings(QObject *parent = nullptr) : QObject(parent) {}
    QString appName() const { return "MyApp"; }
    int version() const { return 1; }
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    QQmlApplicationEngine engine;
    QtForge forge;
    forge.setupEngine(&engine);
    
    AppSettings *settings = new AppSettings(&app);
    
    // Регистрируем в root context — теперь доступен везде в QML как "appSettings"
    forge.setContextProperty("appSettings", settings);
    
    // Можно регистрировать до или после load() — не важно
    forge.load(QUrl("qrc:/main.qml"));
    return app.exec();
}
```

**main.qml:**
```qml
import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    width: 640
    height: 480
    visible: true
    
    Column {
        anchors.centerIn: parent
        spacing: 10
        
        Text {
            text: "App: " + appSettings.appName
            font.pixelSize: 20
        }
        Text {
            text: "Version: " + appSettings.version
            font.pixelSize: 20
        }
    }
}
```

### Пример 3: Использование привязок (registerBinding)

Привязки — это способ связать C++ объект с конкретным QML объектом. В отличие от root context, привязка работает только с одним объектом, что полезно когда у вас несколько одинаковых компонентов с разными данными.

**Важно:** QML объект **обязательно** должен иметь `objectName`, иначе привязка не сработает.

**main.cpp:**
```cpp
#include <QtForge.h>
#include <QQmlApplicationEngine>
#include <QGuiApplication>
#include <QObject>

class DataProvider : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString data READ data NOTIFY dataChanged)
    
public:
    explicit DataProvider(QObject *parent = nullptr) : QObject(parent), m_data("Initial Data") {}
    
    QString data() const { return m_data; }
    void setData(const QString &data) {
        if (m_data != data) {
            m_data = data;
            emit dataChanged();
        }
    }
    
signals:
    void dataChanged();
    
private:
    QString m_data;
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    QQmlApplicationEngine engine;
    QtForge forge;
    forge.setupEngine(&engine);
    
    // Создаем провайдер данных
    DataProvider *provider = new DataProvider(&app);
    
    // Регистрируем привязку ДО загрузки QML
    // Первый параметр — objectName из QML, второй — имя свойства, третий — C++ объект
    forge.registerBinding("myComponent", "dataSource", provider);
    
    // Можно зарегистрировать несколько привязок для разных объектов
    DataProvider *provider2 = new DataProvider(&app);
    provider2->setData("Second Provider");
    forge.registerBinding("anotherComponent", "dataSource", provider2);
    
    // Привязки применяются автоматически после загрузки QML
    forge.load(QUrl("qrc:/main.qml"));
    return app.exec();
}
```

**main.qml:**
```qml
import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    width: 640
    height: 480
    visible: true
    
    Column {
        anchors.centerIn: parent
        spacing: 20
        
        // ВАЖНО: без objectName привязка не сработает!
        Rectangle {
            id: myComponent
            objectName: "myComponent"  // Это имя должно совпадать с первым параметром registerBinding
            width: 200
            height: 100
            color: "lightblue"
            
            // Свойство dataSource будет установлено автоматически после загрузки
            // Проверка на null нужна, потому что привязка применяется асинхронно
            property var dataSource: null
            
            Text {
                anchors.centerIn: parent
                text: dataSource ? dataSource.data : "No data"
            }
        }
        
        Rectangle {
            id: anotherComponent
            objectName: "anotherComponent"
            width: 200
            height: 100
            color: "lightgreen"
            
            Text {
                anchors.centerIn: parent
                text: dataSource ? dataSource.data : "No data"
            }
        }
    }
}
```

### Пример 4: Работа с локальными контекстами

Локальные контексты — это способ изолировать данные между компонентами. Полезно когда у вас несколько одинаковых компонентов, но с разными данными (например, карточки пользователей). В отличие от root context, каждый локальный контекст видит только свои данные. TODO Показать работу с несколькими свойствами

**main.cpp:**
```cpp
#include <QtForge.h>
#include <QQmlApplicationEngine>
#include <QGuiApplication>
#include <QObject>

class UserData : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(int age READ age CONSTANT)
    
public:
    UserData(const QString &name, int age, QObject *parent = nullptr)
        : QObject(parent), m_name(name), m_age(age) {}
    
    QString name() const { return m_name; }
    int age() const { return m_age; }
    
private:
    QString m_name;
    int m_age;
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    QQmlApplicationEngine engine;
    QtForge forge;
    forge.setupEngine(&engine);
    
    // Создаем локальные контексты — каждый получит свой UUID
    QString context1 = forge.createLocalContext();
    QString context2 = forge.createLocalContext();
    
    // Сохраните эти ID, они понадобятся для applyContextToObject
    // На практике я обычно храню их в map или структуре данных
    
    // Создаем данные для каждого контекста
    UserData *user1 = new UserData("Alice", 25, &app);
    UserData *user2 = new UserData("Bob", 30, &app);
    
    // Устанавливаем свойства в локальные контексты
    // Обратите внимание — имя свойства одинаковое ("user"), но контексты разные
    forge.setContextProperty("user", user1, context1);
    forge.setContextProperty("user", user2, context2);
    
    // Применяем контексты к QML объектам ДО загрузки QML
    // Если вызвать после load(), объекты могут не найтись
    forge.applyContextToObject("userCard1", context1);
    forge.applyContextToObject("userCard2", context2);
    
    forge.load(QUrl("qrc:/main.qml"));
    return app.exec();
}
```

**main.qml:**
```qml
import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    width: 640
    height: 480
    visible: true
    
    Row {
        anchors.centerIn: parent
        spacing: 20
        
        // Первая карточка пользователя с локальным контекстом
        Rectangle {
            id: userCard1
            objectName: "userCard1"  // Важно для applyContextToObject
            width: 200
            height: 150
            color: "lightblue"
            border.color: "blue"
            border.width: 2
            
            Column {
                anchors.centerIn: parent
                spacing: 5
                Text { text: "User 1" }
                Text { text: "Name: " + (user ? user.name : "N/A") }
                Text { text: "Age: " + (user ? user.age : "N/A") }
            }
        }
        
        // Вторая карточка с другим локальным контекстом
        Rectangle {
            id: userCard2
            objectName: "userCard2"
            width: 200
            height: 150
            color: "lightgreen"
            border.color: "green"
            border.width: 2
            
            Column {
                anchors.centerIn: parent
                spacing: 5
                Text { text: "User 2" }
                Text { text: "Name: " + (user ? user.name : "N/A") }
                Text { text: "Age: " + (user ? user.age : "N/A") }
            }
        }
    }
}
```

### Пример 5: Регистрация singleton'а

Singleton'ы удобны для глобальных контроллеров, которые нужны везде в приложении (например, менеджер состояния, навигация, API клиент). В отличие от root context property, singleton доступен через импорт модуля, что более явно и типобезопасно.

**main.cpp:**
```cpp
#include <QtForge.h>
#include <QQmlApplicationEngine>
#include <QGuiApplication>
#include <QObject>

class AppController : public QObject {
    Q_OBJECT
    Q_PROPERTY(int counter READ counter NOTIFY counterChanged)
    
public:
    explicit AppController(QObject *parent = nullptr) : QObject(parent), m_counter(0) {}
    
    int counter() const { return m_counter; }
    
    Q_INVOKABLE void increment() {
        m_counter++;
        emit counterChanged();
    }
    
signals:
    void counterChanged();
    
private:
    int m_counter;
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    QQmlApplicationEngine engine;
    QtForge forge;
    forge.setupEngine(&engine);
    
    AppController *controller = new AppController(&app);
    
    // Регистрируем как singleton ДО загрузки QML
    // Первый параметр — имя модуля (может быть любым)
    // Второй — имя singleton'а в QML
    forge.registerSingletonInstance("com.example", "AppController", controller);
    
    // Теперь в QML нужно импортировать: import com.example 1.0
    
    forge.load(QUrl("qrc:/main.qml"));
    return app.exec();
}
```

**main.qml:**
```qml
import QtQuick 2.15
import QtQuick.Window 2.15
import com.example 1.0  // Импортируем модуль с singleton'ом

Window {
    width: 640
    height: 480
    visible: true
    
    Column {
        anchors.centerIn: parent
        spacing: 20
        
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Counter: " + AppController.counter
            font.pixelSize: 24
        }
        
        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Increment"
            onClicked: AppController.increment()
        }
    }
}
```

### Пример 6: Использование PropertyMap

PropertyMap — это способ передать в QML данные, структура которых может меняться во время выполнения. Полезно для конфигураций, настроек, или когда данные приходят извне и вы не знаете заранее, какие поля будут.

**main.cpp:**
```cpp
#include <QtForge.h>
#include <QQmlApplicationEngine>
#include <QGuiApplication>
#include <QTimer>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    QQmlApplicationEngine engine;
    QtForge forge;
    forge.setupEngine(&engine);
    
    // Создаем PropertyMap для динамических данных
    QQmlPropertyMap *dynamicData = forge.registerPropertyMap("dynamicData");
    
    if (dynamicData) {
        dynamicData->insert("temperature", 20);
        dynamicData->insert("humidity", 45);
        dynamicData->insert("status", "OK");
        
        // Симуляция изменения данных
        QTimer *timer = new QTimer(&app);
        QObject::connect(timer, &QTimer::timeout, [dynamicData]() {
            // Динамически обновляем значения
            int temp = dynamicData->value("temperature").toInt();
            dynamicData->insert("temperature", temp + 1);
            
            // Можно добавлять новые свойства на лету
            if (!dynamicData->contains("lastUpdate")) {
                dynamicData->insert("lastUpdate", QTime::currentTime().toString());
            }
        });
        timer->start(1000);
    }
    
    forge.load(QUrl("qrc:/main.qml"));
    return app.exec();
}
```

**main.qml:**
```qml
import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    width: 640
    height: 480
    visible: true
    
    Column {
        anchors.centerIn: parent
        spacing: 15
        
        Text {
            text: "Temperature: " + dynamicData.temperature + "°C"
            font.pixelSize: 20
        }
        
        Text {
            text: "Humidity: " + dynamicData.humidity + "%"
            font.pixelSize: 20
        }
        
        Text {
            text: "Status: " + dynamicData.status
            font.pixelSize: 20
        }
        
        Text {
            text: "Last Update: " + (dynamicData.lastUpdate || "N/A")
            font.pixelSize: 16
            color: "gray"
        }
    }
}
```

### Пример 7: Создание и использование компонентов

Компоненты — это способ программно создавать QML объекты из файлов. Полезно когда нужно динамически создавать элементы интерфейса. Используйте `createAndAddComponent()` — он автоматически добавляет компонент в указанный контейнер и делает его видимым в интерфейсе.

**main.cpp:**
```cpp
#include <QtForge.h>
#include <QQmlApplicationEngine>
#include <QGuiApplication>
#include <QObject>

class ItemData : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    
public:
    ItemData(const QString &title, const QString &desc, QObject *parent = nullptr)
        : QObject(parent), m_title(title), m_description(desc) {}
    
    QString title() const { return m_title; }
    QString description() const { return m_description; }
    
private:
    QString m_title;
    QString m_description;
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    QQmlApplicationEngine engine;
    QtForge forge;
    forge.setupEngine(&engine);
    
    // Загружаем главный QML файл
    forge.load(QUrl("qrc:/main.qml"));
    
    // Получаем root объект из QML
    auto rootObjects = engine.rootObjects();
    if (rootObjects.isEmpty()) {
        qCritical() << "Failed to load QML";
        return -1;
    }
    
    // Находим контейнер для компонентов по objectName
    QObject *container = rootObjects.first()->findChild<QObject*>("itemsContainer");
    if (!container) {
        qWarning() << "Container 'itemsContainer' not found in QML";
        return -1;
    }
    
    // Создаем компонент из отдельного QML файла
    QString componentId = forge.createComponent(QUrl("qrc:/ItemComponent.qml"));
    
    if (!componentId.isEmpty()) {
        // Создаем данные для компонентов
        ItemData *item1 = new ItemData("Item 1", "Description of item 1", &app);
        ItemData *item2 = new ItemData("Item 2", "Description of item 2", &app);
        ItemData *item3 = new ItemData("Item 3", "Description of item 3", &app);
        
        // Создаем и добавляем компоненты в контейнер
        // Каждый вызов создает новый экземпляр и автоматически добавляет его в дерево
        QObject *created1 = forge.createAndAddComponent(componentId, "itemData", item1, container);
        QObject *created2 = forge.createAndAddComponent(componentId, "itemData", item2, container);
        QObject *created3 = forge.createAndAddComponent(componentId, "itemData", item3, container);
        
        if (!created1 || !created2 || !created3) {
            qWarning() << "Failed to create some components";
        }
    }
    
    return app.exec();
}
```

**main.qml:**
```qml
import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    width: 640
    height: 480
    visible: true
    
    Column {
        anchors.centerIn: parent
        spacing: 10
        
        Text {
            text: "Main Window"
            font.pixelSize: 24
        }
        
        // Контейнер для динамически созданных компонентов
        // Важно: должен иметь objectName для поиска из C++
        Column {
            id: itemsContainer
            objectName: "itemsContainer"  // Это имя используется в findChild()
            width: parent.width
            spacing: 10
            // Компоненты будут автоматически добавлены сюда через createAndAddComponent()
        }
    }
}
```

**ItemComponent.qml:**
```qml
import QtQuick 2.15

Rectangle {
    width: 300
    height: 80
    color: "lightblue"
    border.color: "blue"
    border.width: 2
    
    // Свойство itemData будет установлено из C++ через createAndAddComponent()
    property var itemData: null
    
    Column {
        anchors.centerIn: parent
        spacing: 5
        Text {
            text: itemData ? itemData.title : "No title"
            font.pixelSize: 18
            font.bold: true
        }
        Text {
            text: itemData ? itemData.description : "No description"
            font.pixelSize: 14
            color: "gray"
        }
    }
}
```

**Разница между методами:**

- `registerComponentProperty()` — создает объект, но не добавляет в дерево. Объект существует в памяти, но не виден в интерфейсе. Используйте только если вам нужен объект без отображения.

- `createAndAddComponent()` — создает объект и автоматически добавляет в указанный контейнер. Компонент сразу виден в интерфейсе. **Рекомендуемый способ** для большинства случаев.

## Ограничения

- Библиотека работает только с `QQmlApplicationEngine`, не с `QQuickView` или другими движками.
- Привязки работают только с объектами, которые имеют `objectName` — это ограничение QML, не библиотеки.
- Локальные контексты создаются с UUID в качестве ID — их нужно сохранять, если планируете использовать позже.
- PropertyMap не самый быстрый способ передачи данных — для больших объемов данных лучше использовать обычные QObject с свойствами.

## Интеграция в проект

Есть три способа подключить библиотеку к своему проекту. Выбирайте в зависимости от ситуации.

### Способ 1: Через find_package (рекомендуется, если установили в систему)

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick)
find_package(QtForge REQUIRED)

qt_add_executable(MyApp
    main.cpp
)

target_link_libraries(MyApp PRIVATE
    Qt6::Core
    Qt6::Quick
    QtForge::QtForgeLib
)
```

Если CMake не находит библиотеку автоматически:

```cmake
set(QtForge_DIR "/usr/local/lib/cmake/QtForge")
find_package(QtForge REQUIRED)
```

### Способ 2: Через add_subdirectory

Если библиотека лежит рядом с вашим проектом (например, в монорепозитории):

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick)

# Добавляем QtForge как подпроект
add_subdirectory(../QtForge QtForge)

qt_add_executable(MyApp
    main.cpp
)

target_link_libraries(MyApp PRIVATE
    Qt6::Core
    Qt6::Quick
    QtForgeLib  # Прямое имя цели без namespace
)
```

### Способ 3: Через FetchContent

Если хотите автоматически скачать библиотеку из Git (удобно для CI/CD):

```cmake
cmake_minimum_required(VERSION 3.16)
project(MyApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick)

include(FetchContent)

FetchContent_Declare(
    QtForge
    GIT_REPOSITORY https://github.com/yourusername/QtForge.git
    GIT_TAG        v1.0.0  # или main, или конкретный коммит
)

FetchContent_MakeAvailable(QtForge)

qt_add_executable(MyApp
    main.cpp
)

target_link_libraries(MyApp PRIVATE
    Qt6::Core
    Qt6::Quick
    QtForgeLib
)
```

## API Reference

Краткая справка по основным методам. Подробные примеры смотрите выше.

**`void setupEngine(QQmlApplicationEngine *engine)`**  
Инициализирует библиотеку. Вызывайте первым, до всех остальных методов.

**`bool load(const QUrl &url)`**  
Загружает QML файл. Возвращает `false` если не удалось загрузить (проверяйте это).

**`void registerBinding(const QString &objectId, const QString &propertyName, QObject *value)`**  
Привязывает C++ объект к QML объекту по `objectName`. Регистрируйте до `load()`.

**`bool setContextProperty(const QString &name, QObject *value, const QString &contextId = "")`**  
Устанавливает свойство в контексте. Если `contextId` пустой — в root context, иначе в локальный.

**`QString createLocalContext(QObject *parent = nullptr)`**  
Создает локальный контекст. Возвращает UUID — сохраните его, понадобится для `applyContextToObject()`.

**`bool applyContextToObject(const QString &objectName, const QString &contextId)`**  
Применяет локальный контекст к QML объекту. Объект должен иметь `objectName`.

**`QString createComponent(const QUrl &url, QObject *parent = nullptr)`**  
Создает компонент из QML файла. Возвращает ID компонента.

**`bool registerSingletonInstance(const QString &module, const QString &name, QObject *instance)`**  
Регистрирует singleton. В QML импортируйте модуль: `import com.example 1.0`.

**`QQmlPropertyMap* registerPropertyMap(const QString &name, QObject *parent = nullptr)`**  
Создает PropertyMap для динамических данных. Может вернуть `nullptr` — проверяйте.

**`bool registerComponentProperty(const QString &componentId, const QString &propertyName, QObject *value)`**  
Создает объект из компонента и устанавливает свойство. **Внимание:** объект создается, но не добавляется в QML дерево — он существует только в памяти и не виден в интерфейсе. Используйте только для специальных случаев.

**`QObject* createAndAddComponent(const QString &componentId, const QString &propertyName, QObject *value, QObject *parentContainer)`**  
Создает объект из компонента, устанавливает свойство и автоматически добавляет в указанный QML контейнер. Компонент сразу виден в интерфейсе. **Рекомендуемый способ** для работы с компонентами. Возвращает указатель на созданный объект или `nullptr` при ошибке.

## Лицензия

MIT License

Copyright (c) 2026 Sergey Konkov

См. файл [LICENSE](LICENSE) для полного текста лицензии.

## Вклад

Нашли баг или есть идея для улучшения? Создайте issue или отправьте pull request. Буду рад помощи.

## Поддержка

Если что-то не работает или есть вопросы — создайте issue в репозитории. Постараюсь ответить.

