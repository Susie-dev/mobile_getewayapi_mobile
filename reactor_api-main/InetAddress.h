#pragma once
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include<string>
//socket的地址协议类
class InetAddress
{
private:
    sockaddr_in addr_;               //表示地址协议的结构体
public:
    InetAddress();
    InetAddress(const std::string &ip,uint16_t port);             //服务端的socket
    InetAddress(const sockaddr_in addr);               //客户端的socket
    ~InetAddress();

    const char*ip()const;             //返回字符串表示的ip
    uint16_t port()const;             //返回整数表示的端口
    const sockaddr*addr()const;       //返回addr_成员的地址，转换成了sockaddr
    void setaddr(sockaddr_in clientaddr); //设置addr_成员的值


};