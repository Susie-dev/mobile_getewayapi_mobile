#include "RelayServer.h"
#include "DatabaseHelper.h"
#include <iostream>
#include <cstdlib>
#include <signal.h>

RelayServer *relay;               

void Stop(int sig) 
{
  printf("sig=%d\n",sig);
  relay->Stop();
  printf("relay已停止。\n");
  delete relay;
  printf("delete relay。\n");
  
  DatabaseHelper::getInstance().disconnect();
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

  // 初始化 MySQL 连接 (支持通过环境变量配置 HOST)
  const char* dbHost = getenv("MYSQL_HOST");
  if (dbHost == nullptr) {
      dbHost = "127.0.0.1";
  }
  
  if (!DatabaseHelper::getInstance().connect(dbHost, "root", "123456", "cold_chain_db", 3306)) {
      std::cerr << "Failed to connect to database. Server will start without persistence." << std::endl;
  } else {
      DatabaseHelper::getInstance().initTables();
  }

  // 启动 Relay Server，参数：IP, 端口, IO线程数(3), 工作线程数(5)
  relay = new RelayServer(argv[1], atoi(argv[2]), 3, 5);
  relay->Start();
  
  return 0;
}
