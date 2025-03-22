// server.cpp
#include "server.h"
#include "asio_context.h"
#include "http_session.h"
#include <iostream>

using namespace std;
using namespace boost::asio;

Server::Server(boost::asio::io_context& io_context, short port) // 修改参数类型
    : io_context_(io_context),  // 直接使用 io_context
      acceptor_(io_context_, ip::tcp::endpoint(ip::tcp::v4(), port)) // 创建服务器，绑定到指定的端口
{
    {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
        cout << "Server: Listening on port " << port << endl;
    }
    do_accept(); // 开始监听连接请求
}

Server::~Server()
{
    boost::system::error_code ec;
    acceptor_.close(ec); // 关闭接受器

    if (ec)
    {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
        cerr << "Server: Error closing acceptor: " << ec.message() << endl;
    }
    else
    {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
        cout << "Server: Acceptor shut down." << endl;
    }
}

void Server::do_accept()
{
    acceptor_.async_accept(
        [this](boost::system::error_code ec, ip::tcp::socket socket)
        { // Lambda表达式：异步接受连接的回调函数
            if (!ec)
            { // 如果没有错误，则表示成功接受了新的连接
                try
                {
                  std::shared_ptr<HttpSession> session = create_session(std::move(socket)); // 创建一个新的会话
                    if (session)
                    { // 如果会话创建成功，则启动会话
                        session->start();
                    }
                    else
                    { // 如果会话创建失败，则输出错误信息
                      std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                        cerr << "Server: Failed to create session." << endl;
                    }
                }
                catch (const std::exception &e)
                {
                    std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                    cerr << "Server: Exception during session creation: " << e.what() << endl;
                }
            }
            else
            { // 如果接受连接时发生错误，则输出错误信息
                if (ec != boost::asio::error::operation_aborted)
                { // Ignore operation_aborted error during shutdown
                  std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                    cerr << "Server: Accept error: " << ec.message() << endl;
                }
            }
            do_accept(); // 再次调用 do_accept，继续监听新的连接
        });
}
void Server::run() {
    io_context_.run();
}