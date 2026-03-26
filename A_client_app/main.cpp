#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "DataSimulator.h"
#include "OrderDatabase.h"
#include "OrderModel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // 1. 初始化数据库
    OrderDatabase db;
    if (db.initDatabase()) {
        db.insertMockData(); // 插入模拟订单
    }

    // 2. 注册 C++ 类型到 QML
    qmlRegisterType<DataSimulator>("ClientApp", 1, 0, "DataSimulator");
    qmlRegisterType<OrderModel>("ClientApp", 1, 0, "OrderModel");

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("ClientApp", "Main");

    return app.exec();
}
