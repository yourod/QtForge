#include <gtest/gtest.h>

#include <QQmlApplicationEngine>

#include "QtForge.h"
#include "helpers/qtforge_test_utils.h"

namespace {

using qtforge_test::TestQmlUrl;

TEST(QtForge_ComponentPropertyRegistration, WhenEngineIsNotConfigured_Expect_RegisterComponentPropertyReturnsFalse)
{
    QtForge forge;
    QObject value;

    EXPECT_FALSE(forge.registerComponentProperty(QStringLiteral("component-id"), QStringLiteral("payload"), &value));
}

TEST(QtForge_ComponentPropertyRegistration, WhenValuePointerIsNull_Expect_RegisterComponentPropertyReturnsFalse)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    forge.setupEngine(&engine);

    EXPECT_FALSE(forge.registerComponentProperty(QStringLiteral("component-id"), QStringLiteral("payload"), nullptr));
}

TEST(QtForge_ComponentPropertyRegistration, WhenComponentIdIsUnknown_Expect_RegisterComponentPropertyReturnsFalse)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    QObject value;
    forge.setupEngine(&engine);

    EXPECT_FALSE(forge.registerComponentProperty(QStringLiteral("missing-component"), QStringLiteral("payload"), &value));
}

TEST(QtForge_ComponentPropertyRegistration, WhenComponentStatusIsNotReady_Expect_RegisterComponentPropertyReturnsFalse)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    QObject value;
    forge.setupEngine(&engine);

    const QString componentId = forge.createComponent(TestQmlUrl(QStringLiteral("component_invalid.qml")));
    ASSERT_FALSE(componentId.isEmpty());
    EXPECT_FALSE(forge.registerComponentProperty(componentId, QStringLiteral("payload"), &value));
}

TEST(QtForge_ComponentPropertyRegistration, WhenComponentCreationFails_Expect_RegisterComponentPropertyReturnsFalse)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    QObject value;
    forge.setupEngine(&engine);

    const QString componentId = forge.createComponent(TestQmlUrl(QStringLiteral("component_required.qml")));
    ASSERT_FALSE(componentId.isEmpty());
    EXPECT_FALSE(forge.registerComponentProperty(componentId, QStringLiteral("payload"), &value));
}

TEST(QtForge_ComponentPropertyRegistration, WhenTargetPropertyDoesNotExist_Expect_RegisterComponentPropertyReturnsFalse)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    QObject value;
    forge.setupEngine(&engine);

    const QString componentId = forge.createComponent(TestQmlUrl(QStringLiteral("component_payload.qml")));
    ASSERT_FALSE(componentId.isEmpty());
    EXPECT_FALSE(forge.registerComponentProperty(componentId, QStringLiteral("missingProperty"), &value));
}

TEST(QtForge_ComponentPropertyRegistration, WhenComponentAndPropertyAreValid_Expect_RegisterComponentPropertyReturnsTrue)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    QObject value;
    forge.setupEngine(&engine);

    const QString componentId = forge.createComponent(TestQmlUrl(QStringLiteral("component_payload.qml")));
    ASSERT_FALSE(componentId.isEmpty());
    EXPECT_TRUE(forge.registerComponentProperty(componentId, QStringLiteral("payload"), &value));
}

} // namespace
