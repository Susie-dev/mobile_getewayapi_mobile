#pragma once
#include <string>
#include <map>
#include <sstream>

class HttpResponse {
public:
    int status_code_ = 200;
    std::string status_message_ = "OK";
    std::string version_ = "HTTP/1.1";
    std::map<std::string, std::string> headers_;
    std::string body_;

    void set_header(const std::string& key, const std::string& value) {
        headers_[key] = value;
    }

    void set_body(const std::string& body) {
        body_ = body;
        set_header("Content-Length", std::to_string(body_.size()));
    }

    std::string to_string() const {
        std::ostringstream oss;
        oss << version_ << " " << status_code_ << " " << status_message_ << "\r\n";
        
        for (const auto& header : headers_) {
            oss << header.first << ": " << header.second << "\r\n";
        }
        oss << "\r\n";
        oss << body_;
        return oss.str();
    }
};
