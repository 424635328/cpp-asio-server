#include "server.h"
#include "asio_context.h" // Include AsioContext header
#include "http_session.h"
#include <iostream>

using namespace std;

Server::Server(AsioContext& io_context, short port)
    : io_context_(io_context.get_io_context()), // Use get_io_context() to get the reference
      acceptor_(io_context_.get_executor(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) { // Pass the executor to acceptor
    // 构造函数：创建服务器，绑定到指定端口
    {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); // Get the lock
        cout << "Server::Server - 创建服务器，端口: " << port << endl;
    }
    do_accept(); // 开始监听连接请求
    {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); // Get the lock
        cout << "服务器监听Port: " << port << endl;
    }
}

Server::~Server() {
    // 析构函数：服务器关闭时执行
    {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); // Get the lock
        cout << "Server::~Server - Shutting down the server..." << endl;
    }
    acceptor_.close(); // 关闭接受器
    {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); // Get the lock
        cout << "Server::~Server - Server shut down." << endl;
    }
}

void Server::do_accept() {
    // 异步接受新的连接
    {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); // Get the lock
        cout << "Server::do_accept - Start receiving loop." << endl;
    }
    acceptor_.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
            // Lambda表达式：异步接受连接的回调函数
            {
                std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); // Get the lock
                cout << "Server::do_accept - async_accept Callback - start" << endl;
            }
            try {
                if (!ec) {
                    // 如果没有错误，则表示成功接受了新的连接
                    {
                        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); // Get the lock
                        cout << "Server::do_accept - Received a new connection." << endl;
                        cout << "Server::do_accept - Socket 本地端点: " << socket.local_endpoint() << endl;
                        cout << "Server::do_accept - Socket 远程端点: " << socket.remote_endpoint() << endl;
                    }
                    auto session = create_session(std::move(socket)); // 创建一个新的会话
                    if (session) {
                        // 如果会话创建成功，则启动会话
                        {
                            std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); // Get the lock
                            cout << "Server::do_accept - Session created, start session." << endl;
                        }
                        session->start();
                    } else {
                        // 如果会话创建失败，则输出错误信息
                        {
                            std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); // Get the lock
                            cerr << "Server::do_accept - 创建会话失败." << endl;
                        }
                    }
                } else {
                    // 如果接受连接时发生错误，则输出错误信息
                    {
                        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); // Get the lock
                        cerr << "Server::do_accept - 接受错误: " << ec.message() << endl;
                        cerr << "Server::do_accept - 接受错误类别: " << ec.category().name() << endl;
                        cerr << "Server::do_accept - 接受错误值: " << ec.value() << endl;
                    }
                }
            } catch (const std::exception& e) {
                {
                    std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); // Get the lock
                    cerr << "Server::do_accept - 异常: " << e.what() << endl;
                }
            }
            {
                std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); // Get the lock
                cout << "Server::do_accept - async_accept 回调 - 结束" << endl;
            }
            do_accept(); // 再次调用 do_accept，继续监听新的连接
        });
    {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); // Get the lock
        cout << "Server::do_accept - async_accept Called" << endl;
    }
}