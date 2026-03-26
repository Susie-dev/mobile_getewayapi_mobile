# High-Performance API Gateway (基于 C++11 Reactor 模型)

这是一个基于 C++11 从零手写的高性能、异步非阻塞 API 网关。底层采用了经典的 **One Loop Per Thread** (类似 Muduo/Nginx) 网络模型，专为微服务架构提供高并发的流量接入与治理能力。

## 🌟 核心特性 (Features)

*   **全异步非阻塞转发**: 实现了基于 `epoll` 的异步 `TcpClient`，网关在代理请求时绝不阻塞，轻松应对 C10K 问题。
*   **智能路由与负载均衡**: 内置 `Router` 组件，支持基于 URL 前缀的路由分发，实现了 `Round-Robin` (轮询) 负载均衡算法。
*   **插件化流量治理链 (Filter Chain)**:
    *   **鉴权拦截 (Auth Filter)**: 统一处理 HTTP Header 级别的 JWT/Token 鉴权，保护后端微服务。
    *   **高并发限流 (Rate Limit Filter)**: 手写基于**令牌桶 (Token Bucket)** 算法的限流器，有效防止雪崩与 DDoS。
*   **优雅的粘包处理**: 自定义 `Buffer` 类，基于状态机实现了 `\r\n\r\n` 零拷贝协议拆包。

## 🏗️ 系统架构图

```text
[ 客户端 (Browser/App) ] 
       │ 
       ▼ (HTTP/1.1)
┌──────────────────────────────────────────────┐
│                API Gateway                   │
│                                              │
│  1. epoll 主事件循环 (接收连接)              │
│       │                                      │
│  2. epoll 从事件循环池 (解析 HTTP)           │
│       │                                      │
│  3. Filter Chain (责任链插件)                │
│      ├── Auth Filter (鉴权拦截)              │
│      └── RateLimit Filter (令牌桶限流)       │
│       │                                      │
│  4. Router & LoadBalancer (服务发现)         │
│       │                                      │
│  5. 异步 TcpClient Pool (非阻塞转发)         │
└───────┼──────────────────────────────────────┘
        │ 
        ▼
[ 后端微服务集群 (Upstream) ]
 ├── User Service (Node 1 / Node 2)
 └── Order Service 
```

## 🚀 快速开始

### 1. 编译环境要求
*   Linux (推荐 Ubuntu 20.04+)
*   GCC/G++ 支持 C++11 标准
*   Make

### 2. 编译与运行
```bash
# 编译网关
make clean && make

# 启动网关服务 (监听 5005 端口)
./reactor 127.0.0.1 5005
```

### 3. 启动 Mock 微服务 (用于测试转发)
在新的终端中运行提供的 Python 测试桩：
```bash
python3 mock_backend.py 8081 &
python3 mock_backend.py 8082 &
```

### 4. 发起请求测试
**测试负载均衡:**
```bash
curl -s http://127.0.0.1:5005/api/user
# 多次请求会交替返回 Node 1 (8081) 和 Node 2 (8082) 的数据
```

**测试高可用防护 (限流/鉴权):**
如果在 `InitFilters` 中开启了鉴权插件，需携带 Token 访问：
```bash
curl -H "Authorization: Bearer trae-secret-token" -s http://127.0.0.1:5005/api/user
```

## 📊 性能压测 (Benchmark)

使用 `wrk` 工具进行压测。在关闭控制台 IO 并调大限流桶容量后，网关自身的转发延迟在**微秒级**。
由于采用全异步转发，整体 QPS 主要受限于后端微服务的处理能力，网关本身极具弹性。

## 📜 代码结构导读
*   `EventLoop / Epoll / Channel`: 封装 Linux epoll，实现事件驱动引擎。
*   `TcpServer / Connection`: 管理下游客户端的接入与会话。
*   `TcpClient`: **(核心)** 管理上游微服务的异步连接与数据透传。
*   `GatewayServer.cpp`: 业务入口，串联路由、插件与转发逻辑。
*   `Filters.h`: 鉴权与限流插件实现。
