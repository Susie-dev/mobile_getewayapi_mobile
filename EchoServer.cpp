#include"EchoServer.h"

EchoServer::EchoServer(const std::string &ip,const uint16_t port,int subthreadnum,int workthreadnum)
           :tcpserver_(ip,port,subthreadnum),threadpool_(workthreadnum,"WORKS")
{
    //以下代码不是必须的,业务关心什么事件,就指定相应的回调函数
  tcpserver_.setnewconnectioncb(std::bind(&EchoServer::HandleNewConnection, this, std::placeholders::_1));
  tcpserver_.setcloseconnectioncb(std::bind(&EchoServer::HandleClose, this, std::placeholders::_1));
  tcpserver_.seterrorconnectioncb(std::bind(&EchoServer::HandleError, this, std::placeholders::_1));
  tcpserver_.setonmessagecb(std::bind(&EchoServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
  tcpserver_.setsendcompletecb(std::bind(&EchoServer::HandleSendComplete, this, std::placeholders::_1));
  tcpserver_.settimeoutcb(std::bind(&EchoServer::HandleTimeOut, this, std::placeholders::_1));
}

EchoServer::~EchoServer()
{

}

void EchoServer::Start()             //启动服务。
{
    tcpserver_.start();
}

void EchoServer::Stop()			         // 停止服务。
{
    // 停止工作线程。
    threadpool_.stop();
    printf("工作线程已停止。\n");
    // 停止IO线程（事件循环）。
    tcpserver_.stop();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EchoServer::HandleNewConnection(spConnection conn)	// 处理新客户端连接请求，在TcpServer类中回调此函数。
{
    //std::cout << "New Connection Come in." << std::endl;
    //printf("EchoServer::HandleNewConnection() thread is %d.\n",syscall(SYS_gettid));
    printf ("%s new connection(fd=%d,ip=%s,port=%d) ok.\n",Timestamp::now().tostring().c_str(),conn->fd(),conn->ip().c_str(),conn->port());
    //根据业务的需求,在这里可以增加其它的代码。
}
void EchoServer::HandleClose(spConnection conn)	// 关闭客户端的连接，在TcpServer类中回调此函数。
{
    printf ("%s new connection closed(fd=%d,ip=%s,port=%d) ok.\n",Timestamp::now().tostring().c_str(),conn->fd(),conn->ip().c_str(),conn->port());
    //std::cout << "EchoServer conn close" << std::endl;
    //根据业务的需求,在这里可以增加其它的代码。
}
void EchoServer::HandleError(spConnection conn)	// 客户端的连接错误，在TcpServer类中回调此函数。
{
   // std::cout << "EchoServer conn error." << std::endl;
    //根据业务的需求,在这里可以增加其它的代码。
}
void EchoServer::HandleMessage(spConnection conn,std::string &message)	// 处理客户端的请求报文，在TcpServer类中回调此函数。
{
    //printf("EchoServer::HandleMessage() thread is %d.\n",syscall(SYS_gettid));
    if(threadpool_.size()==0)
    {
        OnMessage(conn,message);
    }else
    {
        //把业务添加到线程池任务队列中
        threadpool_.addtask(std::bind(&EchoServer::OnMessage,this,conn,message));
    }
}
void EchoServer::OnMessage(spConnection conn,std::string& message)   //处理客户端的请求报文,用于添加给线程池。
{
      // 在这里，将经过若干步骤的运算。
    //printf("%s message (fd=%d):%s\n",Timestamp::now().tostring().c_str(),conn->fd(),message.c_str());  
    message="reply:"+message;                //回显业务  
    conn->send(message.data(),message.size()); //把数据发送出去。   
}

void EchoServer::HandleSendComplete(spConnection conn)	// 数据发送完成后，在TcpServer类中回调此函数。
{
   // std::cout << "Message send complete." << std::endl;
    //根据业务的需求,在这里可以增加其它的代码。
}
void EchoServer::HandleTimeOut(EventLoop *loop)		// epoll_wait()超时，在TcpServer类中回调此函数。
{
    //std::cout << "EchoServer timeout." <<std::endl;
    //根据业务的需求,在这里可以增加其它的代码。
}
