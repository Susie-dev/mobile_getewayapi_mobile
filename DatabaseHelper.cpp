#include "DatabaseHelper.h"
#include <iostream>
#include <sstream>

DatabaseHelper::DatabaseHelper() : m_mysql(nullptr), m_isConnected(false) {
}

DatabaseHelper::~DatabaseHelper() {
    disconnect();
}

bool DatabaseHelper::connect(const std::string& host, const std::string& user, 
                             const std::string& passwd, const std::string& db_name, int port) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isConnected) return true;

    m_mysql = mysql_init(nullptr);
    if (!m_mysql) {
        std::cerr << "MySQL initialization failed!" << std::endl;
        return false;
    }

    if (!mysql_real_connect(m_mysql, host.c_str(), user.c_str(), passwd.c_str(), 
                            db_name.c_str(), port, nullptr, 0)) {
        std::cerr << "MySQL connection failed: " << mysql_error(m_mysql) << std::endl;
        mysql_close(m_mysql);
        m_mysql = nullptr;
        return false;
    }

    // 设置字符集为 utf8mb4，防止中文乱码
    mysql_set_character_set(m_mysql, "utf8mb4");
    m_isConnected = true;
    std::cout << "MySQL connected successfully to " << db_name << std::endl;
    return true;
}

void DatabaseHelper::disconnect() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isConnected && m_mysql) {
        mysql_close(m_mysql);
        m_mysql = nullptr;
        m_isConnected = false;
        std::cout << "MySQL disconnected." << std::endl;
    }
}

bool DatabaseHelper::initTables() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_isConnected) return false;

    // 1. 创建订单概览表
    const char* create_orders_table = 
        "CREATE TABLE IF NOT EXISTS orders ("
        "order_id VARCHAR(64) PRIMARY KEY, "
        "goods_name VARCHAR(128) NOT NULL, "
        "start_time BIGINT NOT NULL, "
        "status VARCHAR(32) DEFAULT 'IN_TRANSIT'"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
        
    if (mysql_query(m_mysql, create_orders_table)) {
        std::cerr << "Failed to create orders table: " << mysql_error(m_mysql) << std::endl;
        return false;
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
        "INDEX idx_order (order_id), " // 添加索引加快查询速度
        "FOREIGN KEY (order_id) REFERENCES orders(order_id) ON DELETE CASCADE"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";

    if (mysql_query(m_mysql, create_logs_table)) {
        std::cerr << "Failed to create transport_logs table: " << mysql_error(m_mysql) << std::endl;
        return false;
    }

    std::cout << "MySQL tables initialized successfully." << std::endl;
    return true;
}

bool DatabaseHelper::upsertOrder(const std::string& order_id, const std::string& goods_name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_isConnected) return false;

    // 使用 INSERT IGNORE 确保同一订单只插入一次开始时间
    long long current_time = time(nullptr);
    std::string query = "INSERT IGNORE INTO orders (order_id, goods_name, start_time, status) VALUES ('" + 
                        order_id + "', '" + goods_name + "', " + std::to_string(current_time) + ", 'IN_TRANSIT')";
    
    if (mysql_query(m_mysql, query.c_str())) {
        std::cerr << "Failed to upsert order: " << mysql_error(m_mysql) << std::endl;
        return false;
    }
    return true;
}

bool DatabaseHelper::insertTransportLog(const TransportLog& log) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_isConnected) return false;

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

    if (mysql_query(m_mysql, ss.str().c_str())) {
        std::cerr << "Failed to insert log: " << mysql_error(m_mysql) << std::endl;
        return false;
    }
    return true;
}

std::vector<OrderOverview> DatabaseHelper::getAllOrders() {
    std::vector<OrderOverview> orders;
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_isConnected) return orders;

    if (mysql_query(m_mysql, "SELECT order_id, goods_name, start_time, status FROM orders ORDER BY start_time DESC")) {
        std::cerr << "Failed to query orders: " << mysql_error(m_mysql) << std::endl;
        return orders;
    }

    MYSQL_RES* res = mysql_store_result(m_mysql);
    if (!res) return orders;

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
    return orders;
}

std::vector<TransportLog> DatabaseHelper::getOrderHistory(const std::string& order_id) {
    std::vector<TransportLog> logs;
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_isConnected) return logs;

    std::string query = "SELECT timestamp, longitude, latitude, weather, humidity, target_temp, current_temp, is_alert "
                        "FROM transport_logs WHERE order_id = '" + order_id + "' ORDER BY timestamp ASC";

    if (mysql_query(m_mysql, query.c_str())) {
        std::cerr << "Failed to query history: " << mysql_error(m_mysql) << std::endl;
        return logs;
    }

    MYSQL_RES* res = mysql_store_result(m_mysql);
    if (!res) return logs;

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
    return logs;
}
