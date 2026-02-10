#include <gtest/gtest.h>

#include <QQmlApplicationEngine>

#include "QtForge.h"
#include "helpers/qtforge_test_utils.h"

namespace {

using qtforge_test::LoadRootObject;
using qtforge_test::TestQmlUrl;

TEST(QtForge_ComponentCreation, WhenEngineIsNotConfigured_Expect_CreateComponentReturnsEmptyId)
{
    QtForge forge;

    EXPECT_TRUE(forge.createComponent(TestQmlUrl(QStringLiteral("component_payload.qml"))).isEmpty());
}

TEST(QtForge_ComponentCreation, WhenEngineIsConfigured_Expect_CreateComponentReturnsNonEmptyId)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    forge.setupEngine(&engine);

    const QString componentId = forge.createComponent(TestQmlUrl(QStringLiteral("component_payload.qml")));
    EXPECT_FALSE(componentId.isEmpty());
}

TEST(QtForge_ComponentCreation, WhenExplicitParentIsProvided_Expect_CreateComponentStillReturnsNonEmptyId)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    QObject parent;
    forge.setupEngine(&engine);

    const QString componentId = forge.createComponent(TestQmlUrl(QStringLiteral("component_payload.qml")), &parent);
    EXPECT_FALSE(componentId.isEmpty());
}

TEST(QtForge_CreateAndAddComponent, WhenEngineIsNotConfigured_Expect_ReturnsNullptr)
{
    QtForge forge;
    QObject value;
    QObject parentContainer;

    EXPECT_EQ(forge.createAndAddComponent(QStringLiteral("component-id"), QStringLiteral("payload"), &value, &parentContainer), nullptr);
}

TEST(QtForge_CreateAndAddComponent, WhenValuePointerIsNull_Expect_ReturnsNullptr)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    QObject parentContainer;
    forge.setupEngine(&engine);

    EXPECT_EQ(forge.createAndAddComponent(QStringLiteral("component-id"), QStringLiteral("payload"), nullptr, &parentContainer), nullptr);
}

TEST(QtForge_CreateAndAddComponent, WhenParentContainerIsNull_Expect_ReturnsNullptr)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    QObject value;
    forge.setupEngine(&engine);

    EXPECT_EQ(forge.createAndAddComponent(QStringLiteral("component-id"), QStringLiteral("payload"), &value, nullptr), nullptr);
}

TEST(QtForge_CreateAndAddComponent, WhenComponentIdIsUnknown_Expect_ReturnsNullptr)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    QObject value;
    QObject parentContainer;
    forge.setupEngine(&engine);

    EXPECT_EQ(forge.createAndAddComponent(QStringLiteral("missing-id"), QStringLiteral("payload"), &value, &parentContainer), nullptr);
}

TEST(QtForge_CreateAndAddComponent, WhenComponentStatusIsNotReady_Expect_ReturnsNullptr)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    QObject value;
    QObject parentContainer;
    forge.setupEngine(&engine);

    const QString componentId = forge.createComponent(TestQmlUrl(QStringLiteral("component_invalid.qml")));
    ASSERT_FALSE(componentId.isEmpty());
    EXPECT_EQ(forge.createAndAddComponent(componentId, QStringLiteral("payload"), &value, &parentContainer), nullptr);
}

TEST(QtForge_CreateAndAddComponent, WhenParentHasNoQmlContext_Expect_ReturnsNullptr)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    QObject value;
    QObject parentContainer;
    forge.setupEngine(&engine);

    const QString componentId = forge.createComponent(TestQmlUrl(QStringLiteral("component_payload.qml")));
    ASSERT_FALSE(componentId.isEmpty());
    EXPECT_EQ(forge.createAndAddComponent(componentId, QStringLiteral("payload"), &value, &parentContainer), nullptr);
}

TEST(QtForge_CreateAndAddComponent, WhenComponentCreationFails_Expect_ReturnsNullptr)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    QObject value;
    forge.setupEngine(&engine);

    QObject *root = LoadRootObject(forge, engine, QStringLiteral("root_with_targets.qml"));
    ASSERT_NE(root, nullptr);
    QObject *container = root->findChild<QObject *>(QStringLiteral("containerObject"));
    ASSERT_NE(container, nullptr);

    const QString componentId = forge.createComponent(TestQmlUrl(QStringLiteral("component_required.qml")));
    ASSERT_FALSE(componentId.isEmpty());
    EXPECT_EQ(forge.createAndAddComponent(componentId, QStringLiteral("payload"), &value, container), nullptr);
}

TEST(QtForge_CreateAndAddComponent, WhenTargetPropertyDoesNotExist_Expect_ReturnsNullptr)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    QObject value;
    forge.setupEngine(&engine);

    QObject *root = LoadRootObject(forge, engine, QStringLiteral("root_with_targets.qml"));
    ASSERT_NE(root, nullptr);
    QObject *container = root->findChild<QObject *>(QStringLiteral("containerObject"));
    ASSERT_NE(container, nullptr);

    const QString componentId = forge.createComponent(TestQmlUrl(QStringLiteral("component_payload.qml")));
    ASSERT_FALSE(componentId.isEmpty());
    EXPECT_EQ(forge.createAndAddComponent(componentId, QStringLiteral("missingProperty"), &value, container), nullptr);
}

TEST(QtForge_CreateAndAddComponent, WhenComponentAndPropertyAreValid_Expect_ReturnsCreatedObject)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    QObject value;
    forge.setupEngine(&engine);

    QObject *root = LoadRootObject(forge, engine, QStringLiteral("root_with_targets.qml"));
    ASSERT_NE(root, nullptr);
    QObject *container = root->findChild<QObject *>(QStringLiteral("containerObject"));
    ASSERT_NE(container, nullptr);

    const QString componentId = forge.createComponent(TestQmlUrl(QStringLiteral("component_payload.qml")));
    ASSERT_FALSE(componentId.isEmpty());

    QObject *createdObject = forge.createAndAddComponent(componentId, QStringLiteral("payload"), &value, container);
    ASSERT_NE(createdObject, nullptr);
    EXPECT_EQ(createdObject->parent(), container);
    EXPECT_EQ(createdObject->property("payload").value<QObject *>(), &value);
}

} // namespace
