#pragma once
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "Connection.h"
#include <memory>
#include <functional>

// 异步 TCP 客户端，用于网关连接后端微服务
class TcpClient
{
private:
    EventLoop* loop_;                                // 运行在哪个事件循环中
    std::unique_ptr<Socket> clientsock_;             // 客户端的 Socket
    std::unique_ptr<Channel> clientchannel_;         // 对应的 Channel
    spConnection conn_;                              // 连接成功后创建的 Connection 对象

    std::function<void(spConnection)> connectioncb_; // 连接建立成功的回调
    std::function<void(spConnection, std::string&)> onmessagecb_; // 收到后端数据的回调
    std::function<void(spConnection)> closecb_;      // 连接关闭的回调
    std::function<void(spConnection)> errorcb_;      // 连接错误的回调

public:
    TcpClient(EventLoop* loop, const std::string& server_ip, uint16_t server_port);
    ~TcpClient();

    void connect(); // 发起非阻塞连接
    
    // 设置回调函数
    void setconnectioncb(std::function<void(spConnection)> fn) { connectioncb_ = fn; }
    void setonmessagecb(std::function<void(spConnection, std::string&)> fn) { onmessagecb_ = fn; }
    void setclosecb(std::function<void(spConnection)> fn) { closecb_ = fn; }
    void seterrorcb(std::function<void(spConnection)> fn) { errorcb_ = fn; }

private:
    void handlewrite(); // 处理非阻塞 connect 的可写事件
    void removechannel();
};
