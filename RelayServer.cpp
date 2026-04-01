#include "RelayServer.h"
#include "DatabaseHelper.h"
#include <iostream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

RelayServer::RelayServer(const std::string &ip, const uint16_t port, int subthreadnum, int workthreadnum)
           :tcpserver_(ip, port, subthreadnum), threadpool_(workthreadnum, "RELAY_WORKER")
{
  tcpserver_.setnewconnectioncb(std::bind(&RelayServer::HandleNewConnection, this, std::placeholders::_1));
  tcpserver_.setcloseconnectioncb(std::bind(&RelayServer::HandleClose, this, std::placeholders::_1));
  tcpserver_.seterrorconnectioncb(std::bind(&RelayServer::HandleError, this, std::placeholders::_1));
  tcpserver_.setonmessagecb(std::bind(&RelayServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
  tcpserver_.setsendcompletecb(std::bind(&RelayServer::HandleSendComplete, this, std::placeholders::_1));
  tcpserver_.settimeoutcb(std::bind(&RelayServer::HandleTimeOut, this, std::placeholders::_1));
}

RelayServer::~RelayServer()
{
}

void RelayServer::Start()
{
    tcpserver_.start();
}

void RelayServer::Stop()
{
    threadpool_.stop();
    printf("RelayServer Worker threads stopped.\n");
    tcpserver_.stop();
}

void RelayServer::HandleNewConnection(spConnection conn)
{
    printf("%s [Relay] New connection (fd=%d, ip=%s, port=%d) OK.\n", Timestamp::now().tostring().c_str(), conn->fd(), conn->ip().c_str(), conn->port());
    
    // 将新连接加入集合
    std::lock_guard<std::mutex> lock(mutex_);
    all_connections_[conn->fd()] = conn;
}

void RelayServer::HandleClose(spConnection conn)
{
    printf("%s [Relay] Connection closed (fd=%d).\n", Timestamp::now().tostring().c_str(), conn->fd());
    
    // 从集合中移除断开的连接
    std::lock_guard<std::mutex> lock(mutex_);
    all_connections_.erase(conn->fd());
}

void RelayServer::HandleError(spConnection conn)
{
    printf("%s [Relay] Connection error (fd=%d).\n", Timestamp::now().tostring().c_str(), conn->fd());
    
    std::lock_guard<std::mutex> lock(mutex_);
    all_connections_.erase(conn->fd());
}

void RelayServer::HandleMessage(spConnection conn, std::string &message)
{
    // 如果工作线程池为空，直接在 IO 线程处理；否则丢给线程池
    if(threadpool_.size() == 0)
    {
        OnMessage(conn, message);
    }
    else
    {
        threadpool_.addtask(std::bind(&RelayServer::OnMessage, this, conn, message));
    }
}

void RelayServer::OnMessage(spConnection conn, std::string& message)
{
    // 1. 尝试解析 JSON 数据以存入数据库或处理查询指令
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(message), &error);
    if (error.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject root = doc.object();
        
        // 检查是否是业务请求指令
        if (root.contains("action")) {
            QString action = root["action"].toString();
            
            // 处理查询历史轨迹的指令
            if (action == "query_history" && root.contains("order_id")) {
                std::string target_order = root["order_id"].toString().toStdString();
                std::cout << "[Relay] Received query_history for order: " << target_order << std::endl;
                
                // 从 MySQL 捞取该订单的所有轨迹
                std::vector<TransportLog> logs = DatabaseHelper::getInstance().getOrderHistory(target_order);
                
                // 组装成 JSON 数组返回给查询者 (管理员端)
                QJsonArray historyArray;
                for (const auto& log : logs) {
                    QJsonObject logObj;
                    logObj["timestamp"] = (qint64)log.timestamp;
                    logObj["longitude"] = log.longitude;
                    logObj["latitude"] = log.latitude;
                    logObj["weather"] = QString::fromStdString(log.weather);
                    logObj["current_temp"] = log.current_temp;
                    logObj["is_alert"] = log.is_alert == 1;
                    historyArray.append(logObj);
                }
                
                QJsonObject responseObj;
                responseObj["type"] = "history_response";
                responseObj["order_id"] = QString::fromStdString(target_order);
                responseObj["data"] = historyArray;
                
                QJsonDocument respDoc(responseObj);
                QByteArray payload = respDoc.toJson(QJsonDocument::Compact);
                payload.append('\n');
                
                // 只将结果发给请求查询的那个连接
                conn->send(payload.data(), payload.size());
                return; // 查询指令不广播
            }
        }

        // 解析并持久化物联网上报数据
        if (root.contains("order_id") && root.contains("location") && root.contains("cargo_status")) {
            TransportLog log;
            log.order_id = root["order_id"].toString().toStdString();
            log.timestamp = root["timestamp"].toVariant().toLongLong();
            
            QJsonObject loc = root["location"].toObject();
            log.longitude = loc["longitude"].toDouble();
            log.latitude = loc["latitude"].toDouble();
            log.weather = loc["weather"].toString().toStdString();
            log.humidity = loc["outside_humidity"].toString().toStdString();

            QJsonObject cargo = root["cargo_status"].toObject();
            log.target_temp = cargo["target_temp"].toDouble();
            log.current_temp = cargo["current_temp"].toDouble();
            log.is_alert = cargo["is_alert"].toBool() ? 1 : 0;

            // 异步丢给数据库插入，不阻塞当前的线程
            DatabaseHelper::getInstance().upsertOrder(log.order_id, "未知货物 (等待绑定)");
            DatabaseHelper::getInstance().insertTransportLog(log);
        }
    }

    // 2. 广播逻辑：收到任何一个客户端发来的数据，都原封不动地转发给**除了发送者自己以外**的所有客户端
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& pair : all_connections_) {
        spConnection target_conn = pair.second;
        // 不发给自己
        if (target_conn->fd() != conn->fd()) {
            target_conn->send(message.data(), message.size());
        }
    }
}

void RelayServer::HandleSendComplete(spConnection conn)
{
    // 发送完成回调
}

void RelayServer::HandleTimeOut(EventLoop *loop)
{
    // 超时回调
}
