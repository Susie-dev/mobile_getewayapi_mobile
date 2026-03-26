#include"Socket.h"
#include<netinet/tcp.h>
#include <cstdio>
#include <cstdlib>

Socket::Socket(int fd):fd_(fd) //传入一个准备好的fd
{

}     
Socket::~Socket()              //关闭fd_
{
    ::close(fd_);
}

int createnonblocking()
{
    int listensock = socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,IPPROTO_TCP);
    if(listensock < 0) {
     //perror("socket");exit(-1);
     printf("%s:%s:%d listen socket create error:%d\n", __FILE__, __FUNCTION__, __LINE__, errno); exit(-1);
    }
    return listensock;
}
int Socket::fd()const      //返回fd_成员
{
    return fd_;
}
std::string Socket::ip()const      //返回ip_成员
{
    return ip_;
}
uint16_t Socket::port()const      //返回port_成员
{
    return port_;
}

void Socket::setreuseaddr(bool on)    // 设置SO_REUSEADDR选项，true-打开，false-关闭。
{
    int  optval = on ? 1:0;
     // 设置SO_REUSEADDR选项，允许地址重用，解决服务器重启时地址已占用问题
    setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)); 
}   
void Socket::setreuseport(bool on)    // 设置SO_REUSEPORT选项。
{
    int  optval = on ? 1:0;
     // 设置SO_REUSEPORT选项，允许多个套接字绑定到同一端口，提升服务器并发能力
    setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)); 
}
void Socket::settcpnodelay(bool on)   // 设置TCP_NODELAY选项。
{
    int  optval = on ? 1:0;
    // 设置TCP_NODELAY选项，禁用Nagle算法，使数据立即发送，降低交互延迟
    ::setsockopt(fd_,IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}   
void Socket::setkeepalive(bool on)    // 设置SO_KEEPALIVE选项。
{
    int  optval = on ? 1:0;
    // 设置SO_KEEPALIVE选项，启用保活机制，定期检测连接是否存活
     setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)); 
}

void Socket::bind(const InetAddress& servaddr)    // 服务端的socket将调用此函数。
{
    if(::bind(fd_, servaddr.addr(),sizeof(servaddr)) < 0 )
    {
     perror("bind");
     close(fd_);
     exit(-1);
    }
    setipport(servaddr.ip(),servaddr.port());
}

void Socket::setipport(const std::string &ip,uint16_t port)        //设置ip和port成员
{
    ip_=ip;
    port_=port;
}

void Socket::listen(int nn)                   // 服务端的socket将调用此函数。
{
    if(::listen(fd_,nn) != 0)
    {
     perror("listen");
     close(fd_);
     exit(-1);
    }
}
int Socket::accept(InetAddress& clientaddr)      // 客户端的socket将调用此函数。
{
    struct sockaddr_in peeraddr;
    socklen_t len = sizeof(peeraddr);
    int clientsock = accept4(fd_,(struct sockaddr*)&peeraddr,&len,SOCK_NONBLOCK);         
    clientaddr.setaddr(peeraddr);    //客户端的地址和协议



    return clientsock;
}
