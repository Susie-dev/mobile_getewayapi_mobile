#pragma once
#include <string>
#include <mysql/mysql.h>
#include <mutex>
#include <vector>
#include <map>
#include <queue>
#include <condition_variable>

// 订单历史轨迹记录
struct TransportLog {
    std::string order_id;
    long long timestamp;
    double longitude;
    double latitude;
    std::string weather;
    std::string humidity;
    double target_temp;
    double current_temp;
    int is_alert;
};

// 订单概览信息
struct OrderOverview {
    std::string order_id;
    std::string goods_name;
    long long start_time;
    std::string status; // "IN_TRANSIT" 或 "DELIVERED"
};

class DatabaseHelper {
public:
    // 单例模式，保证全局唯一的数据库连接池或句柄
    static DatabaseHelper& getInstance() {
        static DatabaseHelper instance;
        return instance;
    }

    bool connect(const std::string& host, const std::string& user, 
                 const std::string& passwd, const std::string& db_name, int port = 3306, int pool_size = 10);
    
    void disconnect();

    // 初始化表结构
    bool initTables();

    // 插入一条新的物联网上报数据
    bool insertTransportLog(const TransportLog& log);
    
    // 更新或插入订单状态 (收到客户端新的 order_id 时)
    bool upsertOrder(const std::string& order_id, const std::string& goods_name);

    // [供管理端查询] 获取所有订单列表
    std::vector<OrderOverview> getAllOrders();
    
    // [供管理端查询] 获取某个订单的历史轨迹
    std::vector<TransportLog> getOrderHistory(const std::string& order_id);

private:
    DatabaseHelper();
    ~DatabaseHelper();

    MYSQL* getConnection();
    void releaseConnection(MYSQL* conn);

    std::queue<MYSQL*> m_pool;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    bool m_isConnected;
    
    std::string m_host;
    std::string m_user;
    std::string m_passwd;
    std::string m_db_name;
    int m_port;
    int m_poolSize;
};
