#include "RelayServer.h"
#include <iostream>
#include <signal.h>

RelayServer *relay;               

void Stop(int sig) 
{
  printf("sig=%d\n",sig);
  relay->Stop();
  printf("relay已停止。\n");
  delete relay;
  printf("delete relay。\n");
  exit(0);
}

int main(int argc, char *argv[])
{
  if(argc != 3) {
    std::cout << "Usage: ./relay_server <ip> <port>\nExample: ./relay_server 0.0.0.0 8888" << std::endl;
    return -1;
  }
  
  signal(SIGTERM, Stop); 
  signal(SIGINT, Stop); 

  // 启动 Relay Server，参数：IP, 端口, IO线程数(3), 工作线程数(5)
  relay = new RelayServer(argv[1], atoi(argv[2]), 3, 5);
  relay->Start();
  
  return 0;
}
