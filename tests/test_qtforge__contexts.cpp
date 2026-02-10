#include <gtest/gtest.h>

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>

#include "QtForge.h"
#include "helpers/qtforge_test_utils.h"

namespace {

using qtforge_test::LoadRootObject;

TEST(QtForge_ContextManagement, WhenValuePointerIsNull_Expect_SetContextPropertyReturnsFalse)
{
    QtForge forge;

    EXPECT_FALSE(forge.setContextProperty(QStringLiteral("value"), nullptr));
}

TEST(QtForge_ContextManagement, WhenEngineIsNotConfigured_Expect_ContextApiFailsGracefully)
{
    QtForge forge;
    QObject value;

    EXPECT_FALSE(forge.setContextProperty(QStringLiteral("value"), &value));
    EXPECT_TRUE(forge.createLocalContext().isEmpty());
    EXPECT_FALSE(forge.applyContextToObject(QStringLiteral("bindingTarget"), QStringLiteral("missing-context")));
}

TEST(QtForge_ContextManagement, WhenSettingRootContextProperty_Expect_PropertyBecomesAvailable)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    QObject value;
    forge.setupEngine(&engine);

    EXPECT_TRUE(forge.setContextProperty(QStringLiteral("rootValue"), &value));
    EXPECT_EQ(engine.rootContext()->contextProperty(QStringLiteral("rootValue")).value<QObject *>(), &value);
}

TEST(QtForge_ContextManagement, WhenSettingUnknownLocalContext_Expect_SetContextPropertyReturnsFalse)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    QObject value;
    forge.setupEngine(&engine);

    EXPECT_FALSE(forge.setContextProperty(QStringLiteral("localValue"), &value, QStringLiteral("unknown-context")));
}

TEST(QtForge_ContextManagement, WhenCreatingLocalContextWithExplicitParent_Expect_ContextIdIsReturned)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    QObject parent;
    forge.setupEngine(&engine);

    const QString contextId = forge.createLocalContext(&parent);
    EXPECT_FALSE(contextId.isEmpty());
}

TEST(QtForge_ContextManagement, WhenNoRootObjectsAreLoaded_Expect_ApplyContextToObjectReturnsFalse)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    forge.setupEngine(&engine);

    const QString contextId = forge.createLocalContext();
    ASSERT_FALSE(contextId.isEmpty());
    EXPECT_FALSE(forge.applyContextToObject(QStringLiteral("bindingTarget"), contextId));
}

TEST(QtForge_ContextManagement, WhenContextIdIsUnknown_Expect_ApplyContextToObjectReturnsFalse)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    forge.setupEngine(&engine);

    EXPECT_FALSE(forge.applyContextToObject(QStringLiteral("bindingTarget"), QStringLiteral("unknown-context")));
}

TEST(QtForge_ContextManagement, WhenTargetObjectDoesNotExist_Expect_ApplyContextToObjectReturnsFalse)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    forge.setupEngine(&engine);

    QObject *root = LoadRootObject(forge, engine, QStringLiteral("root_with_targets.qml"));
    ASSERT_NE(root, nullptr);

    const QString contextId = forge.createLocalContext();
    ASSERT_FALSE(contextId.isEmpty());
    EXPECT_FALSE(forge.applyContextToObject(QStringLiteral("missingObject"), contextId));
}

TEST(QtForge_ContextManagement, WhenContextAndTargetExist_Expect_ContextIsAppliedToObject)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    forge.setupEngine(&engine);

    QObject localValue;
    const QString contextId = forge.createLocalContext();
    ASSERT_FALSE(contextId.isEmpty());
    ASSERT_TRUE(forge.setContextProperty(QStringLiteral("localValue"), &localValue, contextId));

    QObject *root = LoadRootObject(forge, engine, QStringLiteral("root_with_targets.qml"));
    ASSERT_NE(root, nullptr);

    QObject dynamicTarget(root);
    dynamicTarget.setObjectName(QStringLiteral("dynamicTarget"));

    ASSERT_TRUE(forge.applyContextToObject(QStringLiteral("dynamicTarget"), contextId));

    QObject *target = root->findChild<QObject *>(QStringLiteral("dynamicTarget"));
    ASSERT_NE(target, nullptr);
    QQmlContext *appliedContext = QQmlEngine::contextForObject(target);
    ASSERT_NE(appliedContext, nullptr);
    EXPECT_EQ(appliedContext->contextProperty(QStringLiteral("localValue")).value<QObject *>(), &localValue);
}

} // namespace
