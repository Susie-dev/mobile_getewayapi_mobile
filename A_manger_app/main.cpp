#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtWebEngineQuick>
#include "ManagerNetwork.h"

int main(int argc, char *argv[])
{
    QtWebEngineQuick::initialize();
    QGuiApplication app(argc, argv);

    qmlRegisterType<ManagerNetwork>("ManagerApp", 1, 0, "ManagerNetwork");

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("ManagerApp", "Main");

    return app.exec();
}
