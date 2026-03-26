#pragma once
#include "TcpServer.h"
#include "EventLoop.h"
#include "Connection.h"
#include "ThreadPool.h"

/*
	EchoServer类：回显服务器
	职责分离：EchoServer只关注业务逻辑，底层 IO（事件循环、连接管理）由TcpServer/EventLoop处理，符合 “单一职责原则”；
	异步解耦：通过业务线程池实现 IO 与业务的异步处理，避免 IO 阻塞；
	回调解耦：通过std::function回调接口，EchoServer与TcpServer完全解耦，可替换EchoServer实现其他业务（如 HTTP 服务器），无需修改底层 IO 框架。
	总结：EchoServer是业务逻辑的 “大脑”，通过回调机制串联底层 IO 事件与业务处理，是 Reactor 模型中 “Handler” 角色的典型实现，
	也是上层业务与底层 IO 框架之间的 “粘合剂
*/

class EchoServer
{
private:
	TcpServer tcpserver_;             //
	ThreadPool threadpool_;           //工作线程池
public:
	EchoServer(const std::string &ip,const uint16_t port,int subthreadnum=3,int workthreadnum=5);
	~EchoServer();

	void Start();			         // 启动服务。
	void Stop();			         // 停止服务。
	//参数spConnection：关联具体的客户端连接，通过它可获取连接的 FD、IP、端口等信息，保证业务逻辑能定位到 “具体哪个客户端”。
	void HandleNewConnection(spConnection conn);	             // 处理新客户端连接请求，在TcpServer类中回调此函数。
	void HandleClose(spConnection conn);                    	 // 关闭客户端的连接，在TcpServer类中回调此函数。
	void HandleError(spConnection conn);	                     // 客户端的连接错误，在TcpServer类中回调此函数。
	void HandleMessage(spConnection conn,std::string &message);  // 处理客户端的请求报文，在TcpServer类中回调此函数。
	void HandleSendComplete(spConnection conn);	                 // 数据发送完成后，在TcpServer类中回调此函数。
	void HandleTimeOut(EventLoop *loop);		                 // epoll_wait()超时，在TcpServer类中回调此函数。

	void OnMessage(spConnection conn,std::string& message);      //处理客户端的请求报文,用于添加给线程池。

};