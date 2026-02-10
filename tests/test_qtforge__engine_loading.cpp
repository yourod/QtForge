#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QMetaObject>
#include <QQmlApplicationEngine>

#include "QtForge.h"
#include "helpers/qtforge_test_utils.h"

namespace {

using qtforge_test::LoadRootObject;
using qtforge_test::SpinEvents;
using qtforge_test::SpinUntil;
using qtforge_test::TestQmlUrl;

TEST(QtForge_EngineLoading, WhenEngineIsNotConfigured_Expect_LoadReturnsFalse)
{
    QtForge forge;

    EXPECT_FALSE(forge.load(TestQmlUrl(QStringLiteral("root_with_targets.qml"))));
}

TEST(QtForge_EngineLoading, WhenEnginePointerIsNull_Expect_LoadStillReturnsFalse)
{
    QtForge forge;
    forge.setupEngine(nullptr);

    EXPECT_FALSE(forge.load(TestQmlUrl(QStringLiteral("root_with_targets.qml"))));
}

TEST(QtForge_EngineLoading, WhenEngineIsConfigured_Expect_LoadCreatesRootObject)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    forge.setupEngine(&engine);

    QObject *root = LoadRootObject(forge, engine, QStringLiteral("root_with_targets.qml"));
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->objectName(), QStringLiteral("rootObject"));
}

TEST(QtForge_EngineLoading, WhenObjectCreatedSignalUrlDiffers_Expect_BindingsAreIgnored)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    forge.setupEngine(&engine);

    QObject value;
    forge.registerBinding(QStringLiteral("bindingTarget"), QStringLiteral("payload"), &value);

    engine.load(TestQmlUrl(QStringLiteral("root_with_targets.qml")));
    ASSERT_TRUE(SpinUntil([&engine]() { return !engine.rootObjects().isEmpty(); }));

    QObject *root = engine.rootObjects().first();
    ASSERT_NE(root, nullptr);
    QObject *target = root->findChild<QObject *>(QStringLiteral("bindingTarget"));
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->property("payload").value<QObject *>(), nullptr);
}

TEST(QtForge_EngineLoading, WhenMainQmlIsMissing_Expect_RootObjectIsNotCreated)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    forge.setupEngine(&engine);

    EXPECT_TRUE(forge.load(TestQmlUrl(QStringLiteral("missing_main.qml"))));
    SpinEvents();
    EXPECT_TRUE(engine.rootObjects().isEmpty());

    QCoreApplication::exit(0);
}

TEST(QtForge_EngineLoading, WhenObjectCreationCallbackReceivesNullObject_Expect_ApplicationExitIsRequested)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    forge.setupEngine(&engine);

    const QUrl mainUrl = TestQmlUrl(QStringLiteral("root_with_targets.qml"));
    ASSERT_TRUE(forge.load(mainUrl));

    const bool invoked = QMetaObject::invokeMethod(
        &forge,
        "onObjectCreated",
        Qt::DirectConnection,
        Q_ARG(QObject *, nullptr),
        Q_ARG(QUrl, mainUrl));
    ASSERT_TRUE(invoked);

    QCoreApplication::exit(0);
}

} // namespace
