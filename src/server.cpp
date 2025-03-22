#include "server.h"
#include "asio_context.h"
#include "http_session.h"
#include <iostream>

using namespace std;

Server::Server(AsioContext &io_context, short port)
    : io_context_(io_context.get_io_context()),
      acceptor_(io_context_.get_executor(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{ // 创建服务器，绑定到指定的端口
    std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
    cout << "Server::Server - 创建服务器，端口: " << port << endl;
    do_accept(); // 开始监听连接请求
    cout << "服务器监听Port: " << port << endl;
}

Server::~Server()
{
    std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
    cout << "Server::~Server - Shutting down the server..." << endl;
    acceptor_.close(); // 关闭接受器
    cout << "Server::~Server - Server shut down." << endl;
}

void Server::do_accept()
{
    std::lock_guard<std::mutex> lock(AsioContext::cout_mutex); 
    cout << "Server::do_accept - Start receiving loop." << endl;
    acceptor_.async_accept(                                                         // 异步接受新的连接
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) { // Lambda表达式：异步接受连接的回调函数
            std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);             
            cout << "Server::do_accept - async_accept Callback - start" << endl;
            try
            {
                if (!ec)
                { // 如果没有错误，则表示成功接受了新的连接
                    cout << "Server::do_accept - Received a new connection." << endl;
                    cout << "Server::do_accept - Socket 本地端点: " << socket.local_endpoint() << endl;
                    cout << "Server::do_accept - Socket 远程端点: " << socket.remote_endpoint() << endl;
                    auto session = create_session(std::move(socket)); // 创建一个新的会话
                    if (session)
                    { // 如果会话创建成功，则启动会话
                        cout << "Server::do_accept - Session created, start session." << endl;
                        session->start();
                    }
                    else
                    { // 如果会话创建失败，则输出错误信息
                        cerr << "Server::do_accept - 创建会话失败." << endl;
                    }
                }
                else
                { // 如果接受连接时发生错误，则输出错误信息
                    cerr << "Server::do_accept - 接受错误: " << ec.message() << endl;
                    cerr << "Server::do_accept - 接受错误类别: " << ec.category().name() << endl;
                    cerr << "Server::do_accept - 接受错误值: " << ec.value() << endl;
                }
            }
            catch (const std::exception &e)
            {
                cerr << "Server::do_accept - 异常: " << e.what() << endl;
            }
            cout << "Server::do_accept - async_accept 回调 - 结束" << endl;
            do_accept(); // 再次调用 do_accept，继续监听新的连接
        });
    cout << "Server::do_accept - async_accept Called" << endl;
}