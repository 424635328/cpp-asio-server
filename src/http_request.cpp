#include "http_request.h"
#include <sstream>
#include <iostream>

using namespace std;

// 从输入流读取 HTTP 请求
istream& operator>>(istream& is, HttpRequest& request) {
    string line;

    // 读取请求行
    getline(is, line);
    istringstream request_line(line);
    request_line >> request.method >> request.uri >> request.version;

    cout << "HttpRequest::operator>> - request_line: " << line << endl;
    cout << "HttpRequest::operator>> - 方法: " << request.method << ", URI: " << request.uri << ", 版本: " << request.version << endl;

    // 读取头部
    while (getline(is, line) && line != "\r") {
        size_t colon_pos = line.find(':');
        if (colon_pos != string::npos) {
            string header_name = line.substr(0, colon_pos);
            string header_value = line.substr(colon_pos + 2);
            request.headers[header_name] = header_value;
            cout << "HttpRequest::operator>> - 头部: " << header_name << ": " << header_value << endl;
        }
    }
    cout << "HttpRequest::operator>> - 头部结束." << endl;

    stringstream body_stream;
    body_stream << is.rdbuf();
    request.body = body_stream.str();
    cout << "HttpRequest::operator>> - request_body: " << request.body << endl;
    return is;
}