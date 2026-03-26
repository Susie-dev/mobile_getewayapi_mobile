#include "OrderDatabase.h"

OrderDatabase::OrderDatabase(QObject *parent) : QObject(parent)
{
}

OrderDatabase::~OrderDatabase()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool OrderDatabase::initDatabase()
{
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        m_db = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        m_db = QSqlDatabase::addDatabase("QSQLITE");
        m_db.setDatabaseName("client_orders.db"); // 将在可执行文件同级目录创建 db 文件
    }

    if (!m_db.open()) {
        qDebug() << "Error: connection with database fail:" << m_db.lastError();
        return false;
    }

    // 创建订单表
    QSqlQuery query;
    bool success = query.exec("CREATE TABLE IF NOT EXISTS orders ("
                              "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                              "order_no TEXT UNIQUE, "
                              "goods_name TEXT, "
                              "origin TEXT, "
                              "destination TEXT, "
                              "target_temp REAL, "
                              "status TEXT)"); // 状态：'PENDING' (待接单), 'IN_TRANSIT' (运输中), 'DELIVERED' (已送达)
    
    if (!success) {
        qDebug() << "Create table failed:" << query.lastError();
        return false;
    }
    
    qDebug() << "Database initialized successfully.";
    return true;
}

void OrderDatabase::insertMockData()
{
    QSqlQuery query;
    // 检查是否已有数据，如果没有则插入模拟数据
    query.exec("SELECT COUNT(*) FROM orders");
    if (query.next() && query.value(0).toInt() == 0) {
        qDebug() << "Inserting mock orders...";
        
        query.prepare("INSERT INTO orders (order_no, goods_name, origin, destination, target_temp, status) "
                      "VALUES (?, ?, ?, ?, ?, ?)");
                      
        query.addBindValue("ORD_20260326_001");
        query.addBindValue("山东烟台红富士苹果 5吨");
        query.addBindValue("山东省烟台市栖霞果园");
        query.addBindValue("北京市海淀区新发地冷库");
        query.addBindValue(4.0);
        query.addBindValue("PENDING");
        query.exec();

        query.addBindValue("ORD_20260326_002");
        query.addBindValue("海南章姬草莓 2吨");
        query.addBindValue("海南省三亚市草莓基地");
        query.addBindValue("上海市浦东新区生鲜仓");
        query.addBindValue(2.0);
        query.addBindValue("PENDING");
        query.exec();
        
        query.addBindValue("ORD_20260326_003");
        query.addBindValue("进口智利车厘子 1吨");
        query.addBindValue("广州洋山港海关冷库");
        query.addBindValue("浙江省杭州市总仓");
        query.addBindValue(0.0);
        query.addBindValue("PENDING");
        query.exec();
    }
}
