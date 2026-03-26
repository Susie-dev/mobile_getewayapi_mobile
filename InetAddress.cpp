#include"InetAddress.h"

InetAddress::InetAddress()
{

}

InetAddress::InetAddress(const std::string &ip,uint16_t port)
{
   addr_.sin_family = AF_INET;
   addr_.sin_addr.s_addr = inet_addr(ip.c_str());//接受所有地址的连接
   addr_.sin_port = htons(port);
}

 InetAddress::InetAddress(const sockaddr_in addr):addr_(addr)
{

}
InetAddress::~InetAddress()
{

}
const char*InetAddress::ip()const            //返回字符串表示的ip
{
  return  inet_ntoa(addr_.sin_addr);         //inet_ntoa()的返回值解释const char *
}
uint16_t InetAddress::port()const            //返回整数表示的端口
{
  return ntohs(addr_.sin_port);
}
const sockaddr*InetAddress::addr()const      //返回addr_成员的地址，转换成了sockaddr
{
  return (sockaddr*)&addr_;
}
 void InetAddress::setaddr(sockaddr_in clientaddr) //设置addr_成员的值
 {
  addr_ = clientaddr;
 }