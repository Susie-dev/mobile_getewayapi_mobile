#ifndef ORDERDATABASE_H
#define ORDERDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

class OrderDatabase : public QObject
{
    Q_OBJECT
public:
    explicit OrderDatabase(QObject *parent = nullptr);
    ~OrderDatabase();

    // 初始化数据库并创建表
    bool initDatabase();
    
    // 插入模拟订单
    void insertMockData();

    // 离线缓存：存储网络断开时未能发送的 JSON 数据
    bool insertOfflineLog(const QByteArray &jsonData);

    // 恢复网络时：获取并清空所有离线缓存的数据
    QList<QByteArray> getAndClearOfflineLogs();

private:
    QSqlDatabase m_db;
};

#endif // ORDERDATABASE_H
