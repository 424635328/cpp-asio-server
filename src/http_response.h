#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string>
#include <unordered_map>

using namespace std;

// HttpResponse类，表示HTTP响应
class HttpResponse {
public:
    // 状态码
    int status_code;
    // HTTP版本
    string version = "HTTP/1.1";
    // 头部信息
    unordered_map<string, string> headers;
    // 响应体
    string body;

    // 转换为字符串
    string to_string() const;
};

#endif