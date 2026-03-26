#pragma once
#include "TcpServer.h"
#include "TcpClient.h"
#include "EventLoop.h"
#include "Connection.h"
#include "ThreadPool.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Router.h"
#include "Filters.h"
#include <map>
#include <mutex>
#include <vector>

class GatewayServer
{
private:
	TcpServer tcpserver_;             
	ThreadPool threadpool_;           
    
    // 路由器实例
    Router router_;

    // 插件责任链
    std::vector<std::shared_ptr<HttpFilter>> filters_;

    // 维护 Client 侧 Connection ID 和 到后端 TcpClient 的映射
    std::mutex mutex_;
    std::map<int, std::shared_ptr<TcpClient>> upstream_clients_;

    // 初始化路由表和插件
    void InitRoutes();
    void InitFilters();

public:
	GatewayServer(const std::string &ip, const uint16_t port, int subthreadnum=3, int workthreadnum=5);
	~GatewayServer();

	void Start();			         
	void Stop();			         
	
	void HandleNewConnection(spConnection conn);	             
	void HandleClose(spConnection conn);                    	 
	void HandleError(spConnection conn);	                     
	void HandleMessage(spConnection conn, std::string &message);  
	void HandleSendComplete(spConnection conn);	                 
	void HandleTimeOut(EventLoop *loop);		                 

	void OnMessage(spConnection conn, std::string& message);      

    // Upstream (后端) 的回调函数
    void OnUpstreamConnected(spConnection downstream_conn, spConnection upstream_conn, const std::string& request_data);
    void OnUpstreamMessage(spConnection downstream_conn, spConnection upstream_conn, std::string& message);
    void OnUpstreamClose(spConnection downstream_conn, spConnection upstream_conn);
    void OnUpstreamError(spConnection downstream_conn, spConnection upstream_conn);
};
