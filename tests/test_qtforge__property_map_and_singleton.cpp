#include <gtest/gtest.h>

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlPropertyMap>

#include "QtForge.h"
#include "helpers/qtforge_test_utils.h"

namespace {

using qtforge_test::UniqueModuleName;

TEST(QtForge_SingletonRegistration, WhenInstancePointerIsNull_Expect_RegisterSingletonReturnsFalse)
{
    QtForge forge;

    EXPECT_FALSE(forge.registerSingletonInstance(QStringLiteral("QtForge.Tests"), QStringLiteral("Singleton"), nullptr));
}

TEST(QtForge_SingletonRegistration, WhenEngineIsNotConfigured_Expect_RegisterSingletonReturnsFalse)
{
    QtForge forge;
    QObject instance;

    EXPECT_FALSE(forge.registerSingletonInstance(QStringLiteral("QtForge.Tests"), QStringLiteral("Singleton"), &instance));
}

TEST(QtForge_SingletonRegistration, WhenEngineIsConfigured_Expect_RegisterSingletonReturnsTrue)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    QObject instance;
    forge.setupEngine(&engine);

    const QString moduleName = UniqueModuleName(QStringLiteral("QtForge.Tests.Module"));
    EXPECT_TRUE(forge.registerSingletonInstance(moduleName, QStringLiteral("Singleton"), &instance));
}

TEST(QtForge_PropertyMapRegistration, WhenEngineIsNotConfigured_Expect_RegisterPropertyMapReturnsNullptr)
{
    QtForge forge;

    EXPECT_EQ(forge.registerPropertyMap(QStringLiteral("dynamicMap")), nullptr);
}

TEST(QtForge_PropertyMapRegistration, WhenEngineIsConfiguredAndParentIsDefault_Expect_MapIsRegisteredInRootContext)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    forge.setupEngine(&engine);

    QQmlPropertyMap *propertyMap = forge.registerPropertyMap(QStringLiteral("dynamicMap"));
    ASSERT_NE(propertyMap, nullptr);
    EXPECT_EQ(engine.rootContext()->contextProperty(QStringLiteral("dynamicMap")).value<QObject *>(), propertyMap);
}

TEST(QtForge_PropertyMapRegistration, WhenExplicitParentIsProvided_Expect_PropertyMapUsesProvidedParent)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    QObject parent;
    forge.setupEngine(&engine);

    QQmlPropertyMap *propertyMap = forge.registerPropertyMap(QStringLiteral("dynamicMapWithParent"), &parent);
    ASSERT_NE(propertyMap, nullptr);
    EXPECT_EQ(propertyMap->parent(), &parent);
}

} // namespace
