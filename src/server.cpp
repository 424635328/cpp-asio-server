#include "server.h"
#include "asio_context.h"
#include "http_session.h"
#include <iostream>

using namespace std; 

Server::Server(AsioContext& io_context, short port)
    : acceptor_(io_context.get_io_context(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
      io_context_(io_context) {
    // 构造函数：创建服务器，绑定到指定端口
    cout << "Server::Server - 创建服务器，端口: " << port << endl;
    do_accept(); // 开始监听连接请求
    cout << "服务器监听Port: " << port << endl;
}

Server::~Server() {
    // 析构函数：服务器关闭时执行
    cout << "Server::~Server - Shut down the server." << endl;
    cout << "Server::~Server - All threads have been joined." << endl;
}

void Server::do_accept() {
    // 异步接受新的连接
    cout << "Server::do_accept - Start receiving loop." << endl;
    acceptor_.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
            // Lambda表达式：异步接受连接的回调函数
            cout << "Server::do_accept - async_accept Callback - start" << endl;
            if (!ec) {
                // 如果没有错误，则表示成功接受了新的连接
                cout << "Server::do_accept - Received a new connection." << endl;
                cout << "Server::do_accept - Socket 本地端点: " << socket.local_endpoint() << endl;
                cout << "Server::do_accept - Socket 远程端点: " << socket.remote_endpoint() << endl;
                auto session = create_session(std::move(socket)); // 创建一个新的会话
                if (session) {
                    // 如果会话创建成功，则启动会话
                    cout << "Server::do_accept - Session created, start session." << endl;
                    session->start();
                } else {
                    // 如果会话创建失败，则输出错误信息
                    cerr << "Server::do_accept - 创建会话失败." << endl;
                }
            } else {
                // 如果接受连接时发生错误，则输出错误信息
                cerr << "Server::do_accept - 接受错误: " << ec.message() << endl;
                cerr << "Server::do_accept - 接受错误类别: " << ec.category().name() << endl;
                cerr << "Server::do_accept - 接受错误值: " << ec.value() << endl;
            }
            cout << "Server::do_accept - async_accept 回调 - 结束" << endl;
            do_accept(); // 再次调用 do_accept，继续监听新的连接
        });
    cout << "Server::do_accept - async_accept Called" << endl;
}