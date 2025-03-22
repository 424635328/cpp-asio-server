#ifndef HTTP_SESSION_H
#define HTTP_SESSION_H

#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>
#include <memory>
#include <iostream>
#include "http_request.h"
#include "http_response.h"

class HttpRequestHandler; // 前置声明

class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
    HttpSession(boost::asio::ip::tcp::socket socket, HttpRequestHandler& request_handler);
    ~HttpSession();

    void start();
    void stop();

private:
    void do_read();
    void do_write(std::shared_ptr<HttpResponse> response);  // 修改参数类型
    void handle_request(const HttpRequest& request);

    boost::asio::ip::tcp::socket socket_;
    boost::asio::streambuf buffer_;
    HttpRequestHandler& request_handler_;
};

#endif