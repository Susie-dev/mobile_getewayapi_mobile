
#include"GatewayServer.h"
#include<iostream>
#include<signal.h>

// 1、设置2和15的信号。
// 2、在信号处理函数中停止主从事件循环和工作线程。
// 3、服务程序主动退出。

GatewayServer *gateway;               //信号处理函数无法直接访问局部变量，因此用全局变量

void Stop(int sig) // 信号2和15的处理函数，功能是停止服务程序。
{
  printf("sig=%d\n",sig);
  //调用GatewayServer::Stop()停止服务。
  gateway->Stop();
  printf("gateway已停止。\n");
  delete gateway;
  printf("delete gateway。\n");
  exit(0);
}

int main(int argc,char *argv[])
{
  /*这里检查主程序的入口参数是否合法*/
  if(argc != 3) {
    std::cout<<"请输入./reactor ip地址 端口号"<<std::endl;
    return -1;
  }
  signal(SIGTERM,Stop); // 信号15，系统kill或killall命令默认发送的信号。
  signal(SIGINT,Stop); // 信号2，按Ctrl+C发送的信号。

  gateway = new GatewayServer(argv[1],atoi(argv[2]),3,5);
  gateway->Start();
  
  return 0;
}
