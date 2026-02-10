#include <gtest/gtest.h>

#include <QQmlApplicationEngine>

#include "QtForge.h"
#include "helpers/qtforge_test_utils.h"

namespace {

using qtforge_test::LoadRootObject;
using qtforge_test::SpinEvents;

TEST(QtForge_Bindings, WhenBindingValueIsNull_Expect_BindingIsIgnored)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    forge.setupEngine(&engine);

    forge.registerBinding(QStringLiteral("bindingTarget"), QStringLiteral("payload"), nullptr);

    QObject *root = LoadRootObject(forge, engine, QStringLiteral("root_with_targets.qml"));
    ASSERT_NE(root, nullptr);
    SpinEvents();

    QObject *target = root->findChild<QObject *>(QStringLiteral("bindingTarget"));
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->property("payload").value<QObject *>(), nullptr);
}

TEST(QtForge_Bindings, WhenTargetObjectDoesNotExist_Expect_NoPropertyMutation)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    forge.setupEngine(&engine);

    QObject value;
    forge.registerBinding(QStringLiteral("missingObject"), QStringLiteral("payload"), &value);

    QObject *root = LoadRootObject(forge, engine, QStringLiteral("root_with_targets.qml"));
    ASSERT_NE(root, nullptr);
    SpinEvents();

    QObject *target = root->findChild<QObject *>(QStringLiteral("bindingTarget"));
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->property("payload").value<QObject *>(), nullptr);
}

TEST(QtForge_Bindings, WhenTargetPropertyDoesNotExist_Expect_SetPropertyFailsSafely)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    forge.setupEngine(&engine);

    QObject value;
    forge.registerBinding(QStringLiteral("bindingTarget"), QStringLiteral("readOnlyPayload"), &value);

    QObject *root = LoadRootObject(forge, engine, QStringLiteral("root_with_targets.qml"));
    ASSERT_NE(root, nullptr);
    SpinEvents();

    QObject *target = root->findChild<QObject *>(QStringLiteral("bindingTarget"));
    ASSERT_NE(target, nullptr);
    EXPECT_NE(target->property("readOnlyPayload").value<QObject *>(), &value);
    EXPECT_EQ(target->property("payload").value<QObject *>(), nullptr);
}

TEST(QtForge_Bindings, WhenTargetObjectAndPropertyExist_Expect_PropertyIsBound)
{
    QtForge forge;
    QQmlApplicationEngine engine;
    forge.setupEngine(&engine);

    QObject value;
    forge.registerBinding(QStringLiteral("bindingTarget"), QStringLiteral("payload"), &value);

    QObject *root = LoadRootObject(forge, engine, QStringLiteral("root_with_targets.qml"));
    ASSERT_NE(root, nullptr);
    SpinEvents();

    QObject *target = root->findChild<QObject *>(QStringLiteral("bindingTarget"));
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->property("payload").value<QObject *>(), &value);
}

} // namespace
