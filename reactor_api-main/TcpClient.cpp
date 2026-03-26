#include "TcpClient.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

TcpClient::TcpClient(EventLoop* loop, const std::string& server_ip, uint16_t server_port)
    : loop_(loop), clientsock_(new Socket(createnonblocking())), clientchannel_(new Channel(loop_, clientsock_->fd()))
{
    clientsock_->setipport(server_ip, server_port);
    // 监听写事件，因为非阻塞 connect 成功或失败都会触发写事件
    clientchannel_->setwritecallback(std::bind(&TcpClient::handlewrite, this));
}

TcpClient::~TcpClient()
{
}

void TcpClient::connect()
{
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(clientsock_->port());
    servaddr.sin_addr.s_addr = inet_addr(clientsock_->ip().c_str());

    int ret = ::connect(clientsock_->fd(), (struct sockaddr*)&servaddr, sizeof(servaddr));
    if (ret == 0) {
        // 连接立刻成功（极少见，除非是本机）
        handlewrite();
    } else if (ret < 0 && errno == EINPROGRESS) {
        // 正在连接中，注册写事件到 epoll
        clientchannel_->enablewriting();
    } else {
        printf("TcpClient::connect failed, errno=%d\n", errno);
    }
}

void TcpClient::handlewrite()
{
    // connect 触发了写事件，首先移除写事件监听，避免 busy loop
    clientchannel_->disablewriting();
    clientchannel_->remove();

    // 检查是否真正连接成功
    int err = 0;
    socklen_t len = sizeof(err);
    if (getsockopt(clientsock_->fd(), SOL_SOCKET, SO_ERROR, &err, &len) < 0 || err != 0) {
        printf("TcpClient connect to %s:%d failed.\n", clientsock_->ip().c_str(), clientsock_->port());
        if (errorcb_) errorcb_(nullptr);
        return;
    }

    // 连接成功，把底层 Socket 移交给标准的 Connection 对象去管理读写
    conn_ = std::make_shared<Connection>(loop_, std::move(clientsock_));
    conn_->setclosecallback(closecb_);
    conn_->seterrorcallback(errorcb_);
    conn_->setonmessagecallback(onmessagecb_);
    
    // 通知上层连接建立成功
    if (connectioncb_) connectioncb_(conn_);
}
