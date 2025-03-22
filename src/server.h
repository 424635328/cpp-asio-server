// server.h
#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <memory>
#include "http_session.h" // 添加头文件

class Server {
public:
    Server(boost::asio::io_context& io_context, short port);  // 保持不变
    virtual ~Server(); // 确保析构函数是虚函数

    void run();
protected:
    virtual std::shared_ptr<HttpSession> create_session(boost::asio::ip::tcp::socket socket) = 0; // 修改返回类型
private:
    void do_accept();

    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
};

#endif