#include "http_response.h"
#include <sstream>
#include <iostream>

std::string HttpResponse::to_string() const {
    std::stringstream ss;
    ss << version << " " << status_code << " OK\r\n";
    
    // 避免修改原始对象
    HttpResponse mutableResponse = *this;
    mutableResponse.headers["Content-Length"] = std::to_string(body.size()); // Add Content-Length header

    for (const auto& header : mutableResponse.headers) {
        ss << header.first << ": " << header.second << "\r\n";
    }
    ss << "\r\n";
    ss << body;

    std::string response_str = ss.str();
    std::cout << "HttpResponse::to_string - Response: " << response_str << std::endl;
    return response_str;
}