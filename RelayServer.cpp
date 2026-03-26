#include "RelayServer.h"
#include <iostream>

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
    // 简单的广播逻辑：收到任何一个客户端发来的数据，都原封不动地转发给**除了发送者自己以外**的所有客户端
    // 因为 Client 是发送者，Manager 是接收者。
    
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
