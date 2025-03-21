#ifndef HTTP_SESSION_H
#define HTTP_SESSION_H

#include <boost/asio.hpp>
#include <memory>
#include "http_request_handler.h" // 引入 HTTP 请求处理器
#include "http_request.h"        // 引入 HTTP 请求类
#include "http_response.h"       // 引入 HTTP 响应类

using namespace std;
using namespace boost::asio;

// HttpSession 类，处理 HTTP 会话
class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
    // 构造函数
    HttpSession(ip::tcp::socket socket, HttpRequestHandler& request_handler);
    // 析构函数
    ~HttpSession();

    // 启动会话
    void start();
    // 停止会话
    void stop();

private:
    // 读取数据，异步读取客户端发送的数据
    void do_read();
    // 写入数据，异步发送 HTTP 响应给客户端
    void do_write(const HttpResponse& response);
    // 处理请求，将 HTTP 请求传递给请求处理器进行处理
    void handle_request(const HttpRequest& request);

    // socket 对象，用于与客户端进行通信
    ip::tcp::socket socket_;
    // 请求处理器，负责处理 HTTP 请求
    HttpRequestHandler& request_handler_;
    // 缓冲区，用于存储接收到的数据
    boost::asio::streambuf buffer_;
};

#endif