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

private:
    QSqlDatabase m_db;
};

#endif // ORDERDATABASE_H
