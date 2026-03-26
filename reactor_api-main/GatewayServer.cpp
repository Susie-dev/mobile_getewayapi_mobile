#include "GatewayServer.h"

GatewayServer::GatewayServer(const std::string &ip, const uint16_t port, int subthreadnum, int workthreadnum)
           :tcpserver_(ip, port, subthreadnum), threadpool_(workthreadnum, "GATEWAY_WORKER")
{
  // 初始化路由表和插件
  InitRoutes();
  InitFilters();

  tcpserver_.setnewconnectioncb(std::bind(&GatewayServer::HandleNewConnection, this, std::placeholders::_1));
  tcpserver_.setcloseconnectioncb(std::bind(&GatewayServer::HandleClose, this, std::placeholders::_1));
  tcpserver_.seterrorconnectioncb(std::bind(&GatewayServer::HandleError, this, std::placeholders::_1));
  tcpserver_.setonmessagecb(std::bind(&GatewayServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
  tcpserver_.setsendcompletecb(std::bind(&GatewayServer::HandleSendComplete, this, std::placeholders::_1));
  tcpserver_.settimeoutcb(std::bind(&GatewayServer::HandleTimeOut, this, std::placeholders::_1));
}

void GatewayServer::InitFilters()
{
    // 为了极限压测，我们将限流器的桶容量调大：100万个，每秒产生10万个
    filters_.push_back(std::make_shared<RateLimitFilter>(1000000, 100000.0));
    
    // 暂时注释掉鉴权插件，方便使用 wrk 发送最简单的 GET 请求
    // filters_.push_back(std::make_shared<AuthFilter>());
    
    printf("Gateway Filters Initialized (Optimized for Benchmark).\n");
}

void GatewayServer::InitRoutes()
{
    // 1. 为 /api/user 服务配置负载均衡器（添加两个模拟后端节点）
    auto user_lb = std::make_shared<RoundRobinLoadBalancer>();
    user_lb->add_node(UpstreamNode("127.0.0.1", 8081));
    user_lb->add_node(UpstreamNode("127.0.0.1", 8082));
    router_.register_route("/api/user", user_lb);

    // 2. 为 /api/order 服务配置负载均衡器
    auto order_lb = std::make_shared<RoundRobinLoadBalancer>();
    order_lb->add_node(UpstreamNode("127.0.0.1", 9091));
    router_.register_route("/api/order", order_lb);
    
    printf("Gateway Routes Initialized.\n");
}

GatewayServer::~GatewayServer()
{
}

void GatewayServer::Start()
{
    tcpserver_.start();
}

void GatewayServer::Stop()
{
    threadpool_.stop();
    printf("Gateway Worker threads stopped.\n");
    tcpserver_.stop();
}

void GatewayServer::HandleNewConnection(spConnection conn)
{
    // printf("%s Gateway new connection(fd=%d,ip=%s,port=%d) ok.\n", Timestamp::now().tostring().c_str(), conn->fd(), conn->ip().c_str(), conn->port());
}

void GatewayServer::HandleClose(spConnection conn)
{
    // printf("%s Gateway connection closed(fd=%d).\n", Timestamp::now().tostring().c_str(), conn->fd());
    std::lock_guard<std::mutex> lock(mutex_);
    upstream_clients_.erase(conn->fd()); // 客户端断开时，清理对应的后端连接
}

void GatewayServer::HandleError(spConnection conn)
{
    printf("%s Gateway connection error(fd=%d).\n", Timestamp::now().tostring().c_str(), conn->fd());
}

void GatewayServer::HandleMessage(spConnection conn, std::string &message)
{
    if(threadpool_.size() == 0)
    {
        OnMessage(conn, message);
    }
    else
    {
        threadpool_.addtask(std::bind(&GatewayServer::OnMessage, this, conn, message));
    }
}

void GatewayServer::OnMessage(spConnection conn, std::string& message)
{
    HttpRequest req;
    HttpResponse res;

    // 1. 解析HTTP请求
    if (req.parse(message)) {
        // 关闭 printf 日志，防止高并发下 printf 成为性能瓶颈
        // printf("Gateway Received HTTP Request: %s %s\n", req.method_.c_str(), req.path_.c_str());

        // 1.5 执行插件过滤链 (鉴权、限流)
        bool passed = true;
        for (auto& filter : filters_) {
            if (!filter->doFilter(req, res, conn->ip())) {
                passed = false;
                break; // 如果被拦截，停止后续执行
            }
        }

        if (!passed) {
            // 被插件拦截，直接返回响应
            std::string response_str = res.to_string();
            conn->send(response_str.data(), response_str.size());
            return;
        }

        // 2. 真实的反向代理逻辑：使用路由器进行匹配
        UpstreamNode target_node("", 0);
        
        if (router_.route(req.path_, target_node)) {
            // 路由命中，发起非阻塞的 Upstream 连接
            // printf("Routing to Upstream: %s:%d\n", target_node.ip.c_str(), target_node.port);
            
            // 注意：因为后端的mock server是用的最简单的HTTP Server，它只能处理完整的HTTP请求
            // 为了让它能正常工作，我们需要构造一个标准的 HTTP GET 请求发过去，而不是把原始可能分片的message直接发过去
            std::string proxy_req = req.method_ + " " + req.path_ + " " + req.version_ + "\r\n";
            for(auto& h : req.headers_) {
                proxy_req += h.first + ": " + h.second + "\r\n";
            }
            proxy_req += "\r\n" + req.body_;

            auto upstream_client = std::make_shared<TcpClient>(conn->getloop(), target_node.ip, target_node.port);
            
            upstream_client->setconnectioncb(std::bind(&GatewayServer::OnUpstreamConnected, this, conn, std::placeholders::_1, proxy_req));
            upstream_client->setonmessagecb(std::bind(&GatewayServer::OnUpstreamMessage, this, conn, std::placeholders::_1, std::placeholders::_2));
            upstream_client->setclosecb(std::bind(&GatewayServer::OnUpstreamClose, this, conn, std::placeholders::_1));
            upstream_client->seterrorcb(std::bind(&GatewayServer::OnUpstreamError, this, conn, std::placeholders::_1));

            {
                std::lock_guard<std::mutex> lock(mutex_);
                upstream_clients_[conn->fd()] = upstream_client;
            }

            upstream_client->connect(); // 异步发起连接，此时线程不阻塞，直接返回
            return;
        } else {
            // 404 Not Found
            res.status_code_ = 404;
            res.status_message_ = "Not Found";
            res.set_body("{\"code\": 404, \"msg\": \"Route Not Found in Gateway\"}");
        }
    } else {
        // 解析失败，返回 400 Bad Request
        res.status_code_ = 400;
        res.status_message_ = "Bad Request";
        res.set_body("{\"code\": 400, \"msg\": \"Bad HTTP Request\"}");
    }

    // 3. 只有路由失败或者解析失败时，网关自己直接返回响应
    std::string response_str = res.to_string();
    conn->send(response_str.data(), response_str.size());
}

// ----------------------------------------------------------------------------------
// Upstream 回调函数
// ----------------------------------------------------------------------------------
void GatewayServer::OnUpstreamConnected(spConnection downstream_conn, spConnection upstream_conn, const std::string& request_data)
{
    // printf("Connected to Upstream server. Forwarding request...\n");
    // 连接成功，把下游客户端的请求发给上游后端服务器
    // 为了简单演示，我们只在连接建立后发送一次
    upstream_conn->send(request_data.data(), request_data.size());
}

void GatewayServer::OnUpstreamMessage(spConnection downstream_conn, spConnection upstream_conn, std::string& message)
{
    // printf("Received response from Upstream server. Forwarding to Client...\n");
    // 收到上游服务器的响应，原样转发给下游客户端
    downstream_conn->send(message.data(), message.size());
}

void GatewayServer::OnUpstreamClose(spConnection downstream_conn, spConnection upstream_conn)
{
    // printf("Upstream server closed connection.\n");
    // 如果后端主动断开了，网关也可以把客户端断开 (也可以根据协议选择保留)
}

void GatewayServer::OnUpstreamError(spConnection downstream_conn, spConnection upstream_conn)
{
    // printf("Upstream server connection error.\n");
    // 如果连接后端失败，网关应该给客户端返回 502 Bad Gateway
    HttpResponse res;
    res.status_code_ = 502;
    res.status_message_ = "Bad Gateway";
    res.set_body("{\"code\": 502, \"msg\": \"Bad Gateway: Target Service is down.\"}");
    std::string response_str = res.to_string();
    downstream_conn->send(response_str.data(), response_str.size());
}

void GatewayServer::HandleSendComplete(spConnection conn)
{
    // HTTP短连接可以在发送完毕后主动关闭，但我们这里尝试支持 keep-alive，所以不断开
}

void GatewayServer::HandleTimeOut(EventLoop *loop)
{
    // 超时处理
}
