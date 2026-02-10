#include "helpers/qtforge_test_utils.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QQmlApplicationEngine>
#include <QUuid>

#include "QtForge.h"

namespace qtforge_test {

QUrl TestQmlUrl(const QString &fileName)
{
    const QString fullPath = QStringLiteral(QTFORGE_TEST_QML_DIR) + QStringLiteral("/") + fileName;
    return QUrl::fromLocalFile(fullPath);
}

bool SpinUntil(const std::function<bool()> &predicate, int timeoutMs)
{
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < timeoutMs) {
        if (predicate()) {
            return true;
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }

    return predicate();
}

void SpinEvents(int timeoutMs)
{
    const bool reachedTimeout = SpinUntil([]() { return false; }, timeoutMs);
    (void)reachedTimeout;
}

QObject *LoadRootObject(QtForge &forge, QQmlApplicationEngine &engine, const QString &fileName)
{
    if (!forge.load(TestQmlUrl(fileName))) {
        return nullptr;
    }
    if (!SpinUntil([&engine]() { return !engine.rootObjects().isEmpty(); })) {
        return nullptr;
    }
    return engine.rootObjects().first();
}

QString UniqueModuleName(const QString &prefix)
{
    return prefix + QStringLiteral(".") + QUuid::createUuid().toString(QUuid::WithoutBraces);
}

} // namespace qtforge_test
