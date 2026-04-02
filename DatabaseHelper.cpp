#include "DatabaseHelper.h"
#include <iostream>
#include <sstream>

DatabaseHelper::DatabaseHelper() : m_isConnected(false), m_poolSize(10) {
}

DatabaseHelper::~DatabaseHelper() {
    disconnect();
}

bool DatabaseHelper::connect(const std::string& host, const std::string& user, 
                             const std::string& passwd, const std::string& db_name, int port, int pool_size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isConnected) return true;

    m_host = host;
    m_user = user;
    m_passwd = passwd;
    m_db_name = db_name;
    m_port = port;
    m_poolSize = pool_size;

    for (int i = 0; i < m_poolSize; ++i) {
        MYSQL* conn = mysql_init(nullptr);
        if (!conn) {
            std::cerr << "MySQL initialization failed for connection " << i << std::endl;
            continue;
        }

        if (!mysql_real_connect(conn, m_host.c_str(), m_user.c_str(), m_passwd.c_str(), 
                                m_db_name.c_str(), m_port, nullptr, 0)) {
            std::cerr << "MySQL connection failed: " << mysql_error(conn) << std::endl;
            mysql_close(conn);
            continue;
        }

        mysql_set_character_set(conn, "utf8mb4");
        m_pool.push(conn);
    }

    if (m_pool.empty()) {
        std::cerr << "Failed to create any database connections for the pool." << std::endl;
        return false;
    }

    m_isConnected = true;
    std::cout << "MySQL connected successfully to " << db_name << " with pool size " << m_pool.size() << std::endl;
    return true;
}

void DatabaseHelper::disconnect() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_isConnected) return;

    while (!m_pool.empty()) {
        MYSQL* conn = m_pool.front();
        m_pool.pop();
        if (conn) {
            mysql_close(conn);
        }
    }
    m_isConnected = false;
    std::cout << "MySQL disconnected and connection pool cleared." << std::endl;
}

MYSQL* DatabaseHelper::getConnection() {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_pool.empty()) {
        if (!m_isConnected) return nullptr;
        m_cond.wait(lock);
    }
    MYSQL* conn = m_pool.front();
    m_pool.pop();
    return conn;
}

void DatabaseHelper::releaseConnection(MYSQL* conn) {
    if (!conn) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_pool.push(conn);
    m_cond.notify_one();
}

bool DatabaseHelper::initTables() {
    if (!m_isConnected) return false;
    MYSQL* conn = getConnection();
    if (!conn) return false;

    bool success = true;

    // 1. 创建订单概览表
    const char* create_orders_table = 
        "CREATE TABLE IF NOT EXISTS orders ("
        "order_id VARCHAR(64) PRIMARY KEY, "
        "goods_name VARCHAR(128) NOT NULL, "
        "start_time BIGINT NOT NULL, "
        "status VARCHAR(32) DEFAULT 'IN_TRANSIT'"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
        
    if (mysql_query(conn, create_orders_table)) {
        std::cerr << "Failed to create orders table: " << mysql_error(conn) << std::endl;
        success = false;
    }

    // 2. 创建历史轨迹与环境日志表
    const char* create_logs_table = 
        "CREATE TABLE IF NOT EXISTS transport_logs ("
        "id BIGINT AUTO_INCREMENT PRIMARY KEY, "
        "order_id VARCHAR(64) NOT NULL, "
        "timestamp BIGINT NOT NULL, "
        "longitude DOUBLE NOT NULL, "
        "latitude DOUBLE NOT NULL, "
        "weather VARCHAR(32), "
        "humidity VARCHAR(32), "
        "target_temp DOUBLE, "
        "current_temp DOUBLE, "
        "is_alert INT DEFAULT 0, "
        "INDEX idx_order (order_id), "
        "FOREIGN KEY (order_id) REFERENCES orders(order_id) ON DELETE CASCADE"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";

    if (mysql_query(conn, create_logs_table)) {
        std::cerr << "Failed to create transport_logs table: " << mysql_error(conn) << std::endl;
        success = false;
    }

    if (success) {
        std::cout << "MySQL tables initialized successfully." << std::endl;
    }
    releaseConnection(conn);
    return success;
}

bool DatabaseHelper::upsertOrder(const std::string& order_id, const std::string& goods_name) {
    if (!m_isConnected) return false;
    MYSQL* conn = getConnection();
    if (!conn) return false;

    long long current_time = time(nullptr);
    std::string query = "INSERT IGNORE INTO orders (order_id, goods_name, start_time, status) VALUES ('" + 
                        order_id + "', '" + goods_name + "', " + std::to_string(current_time) + ", 'IN_TRANSIT')";
    
    bool success = true;
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Failed to upsert order: " << mysql_error(conn) << std::endl;
        success = false;
    }
    
    releaseConnection(conn);
    return success;
}

bool DatabaseHelper::insertTransportLog(const TransportLog& log) {
    if (!m_isConnected) return false;
    MYSQL* conn = getConnection();
    if (!conn) return false;

    std::stringstream ss;
    ss << "INSERT INTO transport_logs (order_id, timestamp, longitude, latitude, weather, humidity, target_temp, current_temp, is_alert) VALUES ('"
       << log.order_id << "', "
       << log.timestamp << ", "
       << log.longitude << ", "
       << log.latitude << ", '"
       << log.weather << "', '"
       << log.humidity << "', "
       << log.target_temp << ", "
       << log.current_temp << ", "
       << log.is_alert << ")";

    bool success = true;
    if (mysql_query(conn, ss.str().c_str())) {
        std::cerr << "Failed to insert log: " << mysql_error(conn) << std::endl;
        success = false;
    }
    
    releaseConnection(conn);
    return success;
}

std::vector<OrderOverview> DatabaseHelper::getAllOrders() {
    std::vector<OrderOverview> orders;
    if (!m_isConnected) return orders;
    MYSQL* conn = getConnection();
    if (!conn) return orders;

    if (mysql_query(conn, "SELECT order_id, goods_name, start_time, status FROM orders ORDER BY start_time DESC")) {
        std::cerr << "Failed to query orders: " << mysql_error(conn) << std::endl;
        releaseConnection(conn);
        return orders;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        releaseConnection(conn);
        return orders;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        OrderOverview order;
        order.order_id = row[0] ? row[0] : "";
        order.goods_name = row[1] ? row[1] : "";
        order.start_time = row[2] ? std::stoll(row[2]) : 0;
        order.status = row[3] ? row[3] : "";
        orders.push_back(order);
    }
    mysql_free_result(res);
    releaseConnection(conn);
    return orders;
}

std::vector<TransportLog> DatabaseHelper::getOrderHistory(const std::string& order_id) {
    std::vector<TransportLog> logs;
    if (!m_isConnected) return logs;
    MYSQL* conn = getConnection();
    if (!conn) return logs;

    std::string query = "SELECT timestamp, longitude, latitude, weather, humidity, target_temp, current_temp, is_alert "
                        "FROM transport_logs WHERE order_id = '" + order_id + "' ORDER BY timestamp ASC";

    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Failed to query history: " << mysql_error(conn) << std::endl;
        releaseConnection(conn);
        return logs;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        releaseConnection(conn);
        return logs;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        TransportLog log;
        log.order_id = order_id;
        log.timestamp = row[0] ? std::stoll(row[0]) : 0;
        log.longitude = row[1] ? std::stod(row[1]) : 0.0;
        log.latitude = row[2] ? std::stod(row[2]) : 0.0;
        log.weather = row[3] ? row[3] : "";
        log.humidity = row[4] ? row[4] : "";
        log.target_temp = row[5] ? std::stod(row[5]) : 0.0;
        log.current_temp = row[6] ? std::stod(row[6]) : 0.0;
        log.is_alert = row[7] ? std::stoi(row[7]) : 0;
        logs.push_back(log);
    }
    mysql_free_result(res);
    releaseConnection(conn);
    return logs;
}
