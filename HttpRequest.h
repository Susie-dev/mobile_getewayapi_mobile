#pragma once
#include <string>
#include <map>
#include <sstream>
#include <iostream>

class HttpRequest {
public:
    std::string method_;
    std::string path_;
    std::string version_;
    std::map<std::string, std::string> headers_;
    std::string body_;

    bool parse(const std::string& request_str) {
        std::istringstream iss(request_str);
        std::string line;
        
        // 解析请求行
        if (std::getline(iss, line)) {
            // 处理可能的 \r
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            std::istringstream line_stream(line);
            line_stream >> method_ >> path_ >> version_;
        } else {
            return false;
        }

        // 解析请求头
        while (std::getline(iss, line) && line != "\r" && !line.empty()) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                std::string key = line.substr(0, colon_pos);
                std::string value = line.substr(colon_pos + 1);
                // 去除前面的空格
                size_t start = value.find_first_not_of(" ");
                if (start != std::string::npos) {
                    value = value.substr(start);
                }
                headers_[key] = value;
            }
        }

        // 简化的Body解析（目前假设body已经在字符串里，实际情况需根据Content-Length处理）
        std::ostringstream body_stream;
        body_stream << iss.rdbuf();
        body_ = body_stream.str();

        return true;
    }
};
