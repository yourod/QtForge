#pragma once

#include <functional>

#include <QObject>
#include <QString>
#include <QUrl>

class QQmlApplicationEngine;
class QtForge;

namespace qtforge_test {

QUrl TestQmlUrl(const QString &fileName);
bool SpinUntil(const std::function<bool()> &predicate, int timeoutMs = 1500);
void SpinEvents(int timeoutMs = 50);
QObject *LoadRootObject(QtForge &forge, QQmlApplicationEngine &engine, const QString &fileName);
QString UniqueModuleName(const QString &prefix);

} // namespace qtforge_test
