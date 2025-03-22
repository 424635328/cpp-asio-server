#include "http_session.h"
#include "http_request_handler.h"
#include "http_request.h"
#include "http_response.h"
#include "asio_context.h"
#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>
#include <vector>
#include <memory>

using namespace std;

HttpSession::HttpSession(boost::asio::ip::tcp::socket socket, HttpRequestHandler& request_handler)
    : socket_(std::move(socket)), request_handler_(request_handler)
{
    std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); // 获取互斥锁，保证线程安全
    cout << "HTTP Session created." << endl;
}

HttpSession::~HttpSession() {
    std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); 
    cout << "HTTP Session destroyed." << endl;
}

void HttpSession::start() {
    std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); 
    cout << "HTTP Session started." << endl;
    try {
        do_read(); // 开始读取数据
    } catch (std::exception& e) {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
        cerr << "Exception in HttpSession::start: " << e.what() << endl;
        stop(); // 发生异常，停止会话
    }
}

void HttpSession::stop() {
    boost::system::error_code ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec); // 关闭发送和接收通道
    if (ec) {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); // 获取互斥锁，保证线程安全
        cerr << "Error shutting down socket: " << ec.message() << endl;
    }
    socket_.close(ec); // 关闭socket

    if (ec) {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
        cerr << "Error closing socket: " << ec.message() << endl;
    }
    std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
    cout << "HTTP Session stopped." << endl;
}

void HttpSession::do_read() {
    auto self = shared_from_this(); // 创建 shared_ptr 避免悬空指针
    std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); // 获取互斥锁，保证线程安全
    cout << "HttpSession::do_read - Starting async_read_until" << endl;
    cout << "HttpSession::do_read - async_read_until called" << endl;
    boost::asio::async_read_until(socket_, buffer_, "\r\n\r\n", // 异步读取直到遇到 \r\n\r\n
        [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
            std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); 
            cout << "HttpSession::do_read - async_read_until callback - Started" << endl;
            cout << "HttpSession::do_read - async_read_until callback - Error code: " << ec.message() << endl;
            cout << "HttpSession::do_read - async_read_until callback - Bytes transferred: " << bytes_transferred << endl;
            try {
                if (!ec) {
                    if (bytes_transferred == 0) {
                        cout << "HttpSession::do_read - async_read_until callback - Received 0 bytes. Closing connection." << endl;
                        stop();
                        return;
                    }
                    cout << "HttpSession::do_read - async_read_until callback - Received " << bytes_transferred << " bytes." << endl;
                    istream request_stream(&buffer_); // 使用buffer_初始化输入流
                    HttpRequest request;
                    request_stream >> request;  // 从输入流中解析请求

                    handle_request(request); // 处理请求

                } else if (ec != boost::asio::error::eof) {
                    cerr << "HttpSession::do_read - async_read_until callback - Error reading: " << ec.message() << endl;
                    stop();
                } else {
                    cout << "HttpSession::do_read - async_read_until callback - Connection closed by client." << endl;
                    stop(); 
                }
            } catch (std::exception& e) {
                cerr << "HttpSession::do_read - async_read_until callback - Exception: " << e.what() << endl;
                stop();
            }
            cout << "HttpSession::do_read - async_read_until callback - Finished" << endl;
        });
}

void HttpSession::do_write(std::shared_ptr<HttpResponse> response) {  
    auto self = shared_from_this(); // 创建 shared_ptr 避免悬空指针

    stringstream header_stream; // 创建字符串流
    header_stream << response->version << " " << response->status_code << " OK\r\n"; // 写入状态行

    HttpResponse mutableResponse = *response;
    mutableResponse.headers["Content-Length"] = to_string(response->body.size()); // 设置Content-Length

    for (const auto& header : mutableResponse.headers) { // 遍历header
        header_stream << header.first << ": " << header.second << "\r\n"; // 写入header
    }
    header_stream << "\r\n"; // 写入空行
    string header_str = header_stream.str(); // 转换为string

    std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); 
    cout << "HttpSession::do_write - Sending headers: " << header_str << endl;

    vector<boost::asio::const_buffer> buffers; // 创建buffer数组
    buffers.push_back(boost::asio::buffer(header_str)); // 添加header
    buffers.push_back(boost::asio::buffer(response->body)); // 添加body

    boost::asio::async_write(socket_, buffers, // 异步写入
        [this, self, response](boost::system::error_code ec, size_t bytes_transferred) { // 异步写入回调函数
            std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); 
            cout << "HttpSession::do_write - async_write callback - Started" << endl;
            cout << "HttpSession::do_write - async_write callback - Error code: " << ec.message() << endl;
            cout << "HttpSession::do_write - async_write callback - Bytes transferred: " << bytes_transferred << endl;

            try {
                if (!ec) {
                    cout << "HttpSession::do_write - async_write callback - Sent " << bytes_transferred << " bytes." << endl;
                    do_read(); //  如果是持久连接，则读取下一个请求
                } else {
                    cerr << "HttpSession::do_write - async_write callback - Error writing: " << ec.message() << endl;
                    stop();
                }
            } catch (const std::exception& e) {
                cerr << "HttpSession::do_write - async_write callback - Exception: " << e.what() << endl;
                stop();
            }

            cout << "HttpSession::do_write - async_write callback - Finished" << endl;
        });
}

void HttpSession::handle_request(const HttpRequest& request) {
    std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); // 获取互斥锁，保证线程安全
    cout << "HttpSession::handle_request - Handling request." << endl;
    try {
        std::shared_ptr<HttpResponse> response = std::make_shared<HttpResponse>(request_handler_.handle_request(request)); // 调用请求处理器处理请求
        do_write(response); // 发送响应
    } catch (std::exception& e) {
        cerr << "HttpSession::handle_request - Exception: " << e.what() << endl;
        stop();
    }
}