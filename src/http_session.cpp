#include "http_session.h"
#include "http_request_handler.h"
#include "http_request.h"
#include "http_response.h"
#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

using namespace std;
HttpSession::HttpSession(boost::asio::ip::tcp::socket socket, HttpRequestHandler& request_handler)
    : socket_(std::move(socket)), request_handler_(request_handler), buffer_(8192)  // 初始化成员变量，增加缓冲区大小到8192字节
{
    cout << "HTTP Session created." << endl; // 打印Session创建信息
}

HttpSession::~HttpSession() {
    cout << "HTTP Session destroyed." << endl; // 打印Session销毁信息
}

void HttpSession::start() {
    cout << "HTTP Session started." << endl; // 打印Session启动信息
    try {
        do_read(); // 开始异步读取
    } catch (std::exception& e) {
        cerr << "Exception in HttpSession::start: " << e.what() << endl; // 打印异常信息
        stop(); // 停止Session
    }
}

void HttpSession::stop() {
    boost::system::error_code ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec); // 关闭socket发送和接收通道
    if (ec) {
        cerr << "Error shutting down socket: " << ec.message() << endl; // 打印关闭socket时的错误信息
    }
    socket_.close(ec); // 关闭socket

    if (ec) {
        cerr << "Error closing socket: " << ec.message() << endl; // 打印关闭socket时的错误信息
    }
    cout << "HTTP Session stopped." << endl; // 打印Session停止信息
}

void HttpSession::do_read() {
    auto self = shared_from_this(); // 创建一个指向自身的shared_ptr，防止this指针失效
    cout << "HttpSession::do_read - Starting async_read_until" << endl; // 打印开始异步读取信息
    boost::asio::async_read_until(socket_, buffer_, "\r\n\r\n", // 异步读取，直到遇到"\r\n\r\n"
        [this, self](boost::system::error_code ec, std::size_t bytes_transferred) { // 异步读取完成后的回调函数
            cout << "HttpSession::do_read - async_read_until callback - Started" << endl; // 打印回调函数开始信息
            cout << "HttpSession::do_read - async_read_until callback - Error code: " << ec.message() << endl; // 打印错误信息
            cout << "HttpSession::do_read - async_read_until callback - Bytes transferred: " << bytes_transferred << endl; // 打印读取的字节数
            try {
                if (!ec) { // 如果没有错误
                    cout << "HttpSession::do_read - async_read_until callback - Received " << bytes_transferred << " bytes." << endl; // 打印接收到的字节数
                    istream request_stream(&buffer_); // 从buffer中创建一个输入流
                    HttpRequest request;
                    request_stream >> request;  //  从输入流中解析请求

                    handle_request(request); // 处理请求

                } else if (ec != boost::asio::error::eof) { // 如果发生了错误且不是文件结束错误
                    cerr << "HttpSession::do_read - async_read_until callback - Error reading: " << ec.message() << endl; // 打印读取错误信息
                    stop(); // 停止Session
                } else { // 如果是文件结束错误
                    cout << "HttpSession::do_read - async_read_until callback - Connection closed by client." << endl; // 打印客户端关闭连接信息
                    stop(); // Connection closed by client 关闭连接
                }
            } catch (std::exception& e) {
                cerr << "HttpSession::do_read - async_read_until callback - Exception: " << e.what() << endl; // 打印异常信息
                stop(); // 停止Session
            }
             cout << "HttpSession::do_read - async_read_until callback - Finished" << endl; // 打印回调函数结束信息
        });
     cout << "HttpSession::do_read - async_read_until called" << endl; // 打印异步读取函数调用信息
}

void HttpSession::do_write(const HttpResponse& response) {
    auto self = shared_from_this(); // 创建一个指向自身的shared_ptr，防止this指针失效
    string response_str = response.to_string(); // 将response对象转换为字符串
    cout << "HttpSession::do_write - Sending response: " << response_str << endl; // 打印发送的响应信息

    boost::asio::async_write(socket_, boost::asio::buffer(response_str), // 异步写入数据到socket
        [this, self](boost::system::error_code ec, std::size_t bytes_transferred) { // 异步写入完成后的回调函数
            cout << "HttpSession::do_write - async_write callback - Started" << endl; // 打印回调函数开始信息
            cout << "HttpSession::do_write - async_write callback - Error code: " << ec.message() << endl; // 打印错误信息
            cout << "HttpSession::do_write - async_write callback - Bytes transferred: " << bytes_transferred << endl; // 打印写入的字节数

            try {
                if (!ec) { // 如果没有错误
                    cout << "HttpSession::do_write - async_write callback - Sent " << bytes_transferred << " bytes." << endl; // 打印发送的字节数
                    //Initiate the next read for the next request.  For persistent connections.
                    do_read(); // 开始下一次读取，用于持久连接
                } else { // 如果发生了错误
                    cerr << "HttpSession::do_write - async_write callback - Error writing: " << ec.message() << endl; // 打印写入错误信息
                    stop(); // 停止Session
                }
            } catch (std::exception& e) {
                cerr << "HttpSession::do_write - async_write callback - Exception: " << e.what() << endl; // 打印异常信息
                stop(); // 停止Session
            }
            cout << "HttpSession::do_write - async_write callback - Finished" << endl; // 打印回调函数结束信息
        });
}

void HttpSession::handle_request(const HttpRequest& request) {
    cout << "HttpSession::handle_request - Handling request." << endl; // 打印处理请求信息
    try {
        HttpResponse response = request_handler_.handle_request(request); // 调用请求处理器处理请求并获取响应
        do_write(response); // 发送响应
    } catch (std::exception& e) {
        cerr << "HttpSession::handle_request - Exception: " << e.what() << endl; // 打印异常信息
        stop(); // 停止Session
    }
}