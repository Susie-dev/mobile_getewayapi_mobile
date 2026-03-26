#pragma once
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <chrono>
#include <mutex>
#include <algorithm>

// 插件基类
class HttpFilter {
public:
    virtual ~HttpFilter() = default;
    
    // 返回 true 表示放行，返回 false 表示拦截（并由插件自己填充 res）
    virtual bool doFilter(const HttpRequest& req, HttpResponse& res, const std::string& client_ip) = 0;
};

// ---------------------------------------------------------
// 1. 简易鉴权插件 (Token Auth)
// ---------------------------------------------------------
class AuthFilter : public HttpFilter {
public:
    bool doFilter(const HttpRequest& req, HttpResponse& res, const std::string& client_ip) override {
        // 模拟：只有请求头里带了 Authorization: Bearer trae-secret-token 才能访问
        auto it = req.headers_.find("Authorization");
        if (it == req.headers_.end() || it->second != "Bearer trae-secret-token") {
            res.status_code_ = 401;
            res.status_message_ = "Unauthorized";
            res.set_body("{\"code\": 401, \"msg\": \"Unauthorized: Invalid or missing token\"}");
            return false; // 拦截
        }
        return true; // 放行
    }
};

// ---------------------------------------------------------
// 2. 令牌桶限流插件 (Rate Limiter)
// ---------------------------------------------------------
class RateLimitFilter : public HttpFilter {
private:
    int capacity_;      // 桶的容量 (最大突发并发数)
    int tokens_;        // 当前桶里的令牌数
    double rate_;       // 令牌产生速率 (每秒产生多少个)
    std::chrono::steady_clock::time_point last_time_; // 上次更新时间
    std::mutex mutex_;

public:
    // 默认：桶大小10个，每秒产生5个令牌
    RateLimitFilter(int capacity = 10, double rate = 5.0) 
        : capacity_(capacity), tokens_(capacity), rate_(rate), last_time_(std::chrono::steady_clock::now()) {}

    bool doFilter(const HttpRequest& req, HttpResponse& res, const std::string& client_ip) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = now - last_time_;
        
        // 按照时间流逝补充令牌
        if (elapsed.count() > 0) {
            int new_tokens = static_cast<int>(elapsed.count() * rate_);
            if (new_tokens > 0) {
                tokens_ = std::min(capacity_, tokens_ + new_tokens);
                last_time_ = now;
            }
        }

        // 尝试获取令牌
        if (tokens_ > 0) {
            tokens_--;
            return true; // 放行
        } else {
            // 没有令牌了，触发限流
            res.status_code_ = 429;
            res.status_message_ = "Too Many Requests";
            res.set_body("{\"code\": 429, \"msg\": \"Rate Limit Exceeded. Please try again later.\"}");
            return false; // 拦截
        }
    }
};
