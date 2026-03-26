#pragma once
#include "TcpServer.h"
#include "EventLoop.h"
#include "Connection.h"
#include "ThreadPool.h"
#include <map>
#include <mutex>

/*
  RelayServer 类：物联网数据转发中心
  职责：
  1. 接收 Client 端的 JSON 数据。
  2. 维护所有 Manager 端的连接。
  3. 将接收到的 JSON 广播转发给所有在线的 Manager。
*/
class RelayServer
{
private:
    TcpServer tcpserver_;             
    ThreadPool threadpool_;           

    // 维护所有的连接。使用 mutex 保证多线程安全
    std::mutex mutex_;
    std::map<int, spConnection> all_connections_;

public:
    RelayServer(const std::string &ip, const uint16_t port, int subthreadnum=3, int workthreadnum=5);
    ~RelayServer();

    void Start();			         
    void Stop();			         
    
    void HandleNewConnection(spConnection conn);	             
    void HandleClose(spConnection conn);                    	 
    void HandleError(spConnection conn);	                     
    void HandleMessage(spConnection conn, std::string &message);  
    void HandleSendComplete(spConnection conn);	                 
    void HandleTimeOut(EventLoop *loop);		                 

    void OnMessage(spConnection conn, std::string& message);      
};
