// http_session.cpp
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

HttpSession::HttpSession(boost::asio::ip::tcp::socket socket, HttpRequestHandler& request_handler, std::function<void()> connection_finished_callback)
    : socket_(std::move(socket)), request_handler_(request_handler), connection_finished_callback_(connection_finished_callback)
{
    { // 限定 lock_guard 的作用域
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); // 获取互斥锁，保证线程安全
        cout << "HTTP Session created." << endl;
    }
}

HttpSession::~HttpSession() {
    {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
        cout << "HTTP Session destroyed." << endl;
    }
    if(connection_finished_callback_) {
        connection_finished_callback_(); // 在析构函数中调用回调
    }
}

void HttpSession::start() {
    {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
        cout << "HTTP Session started." << endl;
    }
    try {
        do_read(); // 开始读取数据
    } catch (std::exception& e) {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
        cerr << "Exception in HttpSession::start: " << e.what() << endl;
        stop(); // 发生异常，停止会话
    }
}

void HttpSession::stop() {
    if (!stopped_) {
        stopped_ = true;
        boost::system::error_code ec;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec); // 关闭发送和接收通道
        if (ec && ec != boost::system::errc::not_connected) {
            {
                std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); // 获取互斥锁，保证线程安全
                cerr << "Error shutting down socket: " << ec.message() << endl;
            }
        }
        socket_.close(ec); // 关闭socket

        if (ec) {
            {
                std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                cerr << "Error closing socket: " << ec.message() << endl;
            }
        }
        {
            std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
            cout << "HTTP Session stopped." << endl;
        }

        if (connection_finished_callback_) {
            connection_finished_callback_(); // 调用回调，保证连接计数正确
        }
    }
}

void HttpSession::do_read() {
    auto self = shared_from_this(); // 创建 shared_ptr 避免悬空指针
    {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); // 获取互斥锁，保证线程安全
        cout << "HttpSession::do_read - Starting async_read_until" << endl;
        cout << "HttpSession::do_read - async_read_until called" << endl;
    }

    boost::asio::async_read_until(socket_, buffer_, "\r\n\r\n", // 异步读取直到遇到 \r\n\r\n
        [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
            if (stopped_) return;//检查会话是否已停止

            try {
                if (!ec) {
                    if (bytes_transferred == 0) {
                        {
                            std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                            cout << "HttpSession::do_read - async_read_until callback - Received 0 bytes. Closing connection." << endl;
                        }
                        stop();
                        return;
                    }
                    {
                        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                        cout << "HttpSession::do_read - async_read_until callback - Received " << bytes_transferred << " bytes." << endl;
                    }

                    istream request_stream(&buffer_); // 使用buffer_初始化输入流
                    HttpRequest request;
                    request_stream >> request;  // 从输入流中解析请求

                    handle_request(request); // 处理请求

                } else if (ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset || ec == boost::asio::error::connection_aborted) {
                    // 连接被客户端关闭
                    {
                        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                        cout << "HttpSession::do_read - async_read_until callback - Connection closed by client." << endl;
                    }
                    stop(); // 停止会话
                } else {
                    {
                        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                        cerr << "HttpSession::do_read - async_read_until callback - Error reading: " << ec.message() << endl;
                    }
                    stop();
                }
            } catch (std::exception& e) {
                {
                    std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                    cerr << "HttpSession::do_read - async_read_until callback - Exception: " << e.what() << endl;
                }
                stop();
            }
            {
                std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                cout << "HttpSession::do_read - async_read_until callback - Finished" << endl;
            }
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
    {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
        cout << "HttpSession::do_write - Sending headers: " << header_str << endl;
    }

    vector<boost::asio::const_buffer> buffers; // 创建buffer数组
    buffers.push_back(boost::asio::buffer(header_str)); // 添加header
    buffers.push_back(boost::asio::buffer(response->body)); // 添加body

    boost::asio::async_write(socket_, buffers, // 异步写入
        [this, self, response](boost::system::error_code ec, size_t bytes_transferred) { // 异步写入回调函数
            if (stopped_) return; 
            try {
                if (!ec) {
                    {
                        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                        cout << "HttpSession::do_write - async_write callback - Sent " << bytes_transferred << " bytes." << endl;
                    }
                    do_read(); //  如果是持久连接，则读取下一个请求
                } else {
                    {
                        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                        cerr << "HttpSession::do_write - async_write callback - Error writing: " << ec.message() << endl;
                    }
                    stop();
                }
            } catch (const std::exception& e) {
                {
                    std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                    cerr << "HttpSession::do_write - async_write callback - Exception: " << e.what() << endl;
                }
                stop();
            }
            {
                std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                cout << "HttpSession::do_write - async_write callback - Finished" << endl;
            }
        });
}

void HttpSession::handle_request(const HttpRequest& request) {
    {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); // 获取互斥锁，保证线程安全
        cout << "HttpSession::handle_request - Handling request." << endl;
    }
    try {
        std::shared_ptr<HttpResponse> response = std::make_shared<HttpResponse>(request_handler_.handle_request(request)); // 调用请求处理器处理请求
        do_write(response); // 发送响应
    } catch (std::exception& e) {
        {
            std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
            cerr << "HttpSession::handle_request - Exception: " << e.what() << endl;
        }
        stop();
    }
}