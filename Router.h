#pragma once
#include <string>
#include <vector>
#include <map>
#include <atomic>
#include <mutex>

// 代表一个后端服务器节点
struct UpstreamNode {
    std::string ip;
    uint16_t port;
    int weight; // 权重（预留给加权轮询）
    
    UpstreamNode(std::string i, uint16_t p, int w = 1) : ip(i), port(p), weight(w) {}
};

// 负载均衡策略基类
class LoadBalancer {
public:
    virtual ~LoadBalancer() = default;
    // 添加节点
    virtual void add_node(const UpstreamNode& node) = 0;
    // 获取下一个处理请求的节点
    virtual bool get_next_node(UpstreamNode& out_node) = 0;
};

// 简单的轮询 (Round-Robin) 负载均衡器
class RoundRobinLoadBalancer : public LoadBalancer {
private:
    std::vector<UpstreamNode> nodes_;
    std::atomic<size_t> current_index_;
    std::mutex mutex_; // 保护 nodes_ 的动态修改

public:
    RoundRobinLoadBalancer() : current_index_(0) {}

    void add_node(const UpstreamNode& node) override {
        std::lock_guard<std::mutex> lock(mutex_);
        nodes_.push_back(node);
    }

    bool get_next_node(UpstreamNode& out_node) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (nodes_.empty()) {
            return false;
        }
        
        // 获取当前索引并自增，取模实现循环
        size_t idx = current_index_++;
        out_node = nodes_[idx % nodes_.size()];
        return true;
    }
};

// 路由器：管理不同的 API 路径对应哪些后端的 LoadBalancer
class Router {
private:
    // 路由表映射：URL Prefix -> LoadBalancer
    std::map<std::string, std::shared_ptr<LoadBalancer>> routes_;
    std::mutex mutex_;

public:
    Router() {}

    // 注册一条路由规则
    void register_route(const std::string& path_prefix, std::shared_ptr<LoadBalancer> lb) {
        std::lock_guard<std::mutex> lock(mutex_);
        routes_[path_prefix] = lb;
    }

    // 根据请求路径找到合适的后端节点
    bool route(const std::string& request_path, UpstreamNode& out_node) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 简单的前缀匹配逻辑 (例如: /api/user 匹配 /api/user/info)
        for (auto it = routes_.rbegin(); it != routes_.rend(); ++it) {
            if (request_path.find(it->first) == 0) {
                // 找到了对应的 LoadBalancer，从中取出一个节点
                return it->second->get_next_node(out_node);
            }
        }
        return false; // 没有找到匹配的路由
    }
};
