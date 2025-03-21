#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <unordered_map>
#include <istream>

using namespace std;

// HttpRequest类，用于表示HTTP请求
class HttpRequest {
public:
    // HTTP请求方法
    string method;
    // 统一资源标识符（URI）
    string uri;
    // HTTP版本号
    string version;
    // 头部信息
    unordered_map<string, string> headers;
    // 请求体
    string body;

    // 输入流操作符重载
    friend istream& operator>>(istream& is, HttpRequest& request);
};

#endif