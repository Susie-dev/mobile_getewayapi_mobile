#include"Acceptor.h"

Acceptor::Acceptor(EventLoop *loop,const std::string &ip,const uint16_t port)
          :loop_(loop),sersock_(createnonblocking()),acceptchannel_(loop_,sersock_.fd())
{
  //sersock_ = new Socket(createnonblocking());
  InetAddress servaddr(ip,port);
  
  sersock_.setkeepalive(true);
  sersock_.setreuseaddr(true);
  sersock_.setreuseport(true);
  sersock_.settcpnodelay(true);
  sersock_.bind(servaddr);
  sersock_.listen();

  //acceptchannel_=new Channel(loop_,sersock_.fd());
  acceptchannel_.setreadcallback(std::bind(&Acceptor::newconnection,this));
  acceptchannel_.enablereading();
}
Acceptor::~Acceptor()
{
    //delete sersock_;
    //delete acceptchannel_;
}
/////////////////////////////////////////////////////////////////////////////////////////////////
#include"Connection.h"

void Acceptor::newconnection()           //处理新客户端连接请求
{
  InetAddress clientaddr;
   std::unique_ptr<Socket>clientsock(new Socket(sersock_.accept(clientaddr)));          
  clientsock->setipport(clientaddr.ip(),clientaddr.port());

  newconnectioncb_(std::move(clientsock));     //回调Tcpserver::newconnection().
}

void Acceptor::setnewconnectioncb(std::function<void( std::unique_ptr<Socket>)> fn)    //设置处理新客户端连接请求的回调函数。
{
  newconnectioncb_ = fn;
}