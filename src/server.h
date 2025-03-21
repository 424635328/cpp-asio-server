#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <memory>

// 前向声明 AsioContext 类
class AsioContext;
// 前向声明 HttpSession 类
class HttpSession;  // Forward declaration

class Server {
public:
    // 构造函数：初始化服务器，绑定监听端口
    Server(AsioContext& io_context, short port);
    // 析构函数：释放服务器资源
    virtual ~Server();

protected:
    // 异步接受客户端连接，由派生类实现具体逻辑
    virtual void do_accept(); // Implementation handles accepting connections
    // 创建新的 HttpSession 会话，由派生类实现，并传递 socket
    virtual std::shared_ptr<HttpSession> create_session(boost::asio::ip::tcp::socket socket) = 0;

    // acceptor_ 成员变量：负责监听客户端连接
    boost::asio::ip::tcp::acceptor acceptor_;
    // io_context_ 成员变量：引用 AsioContext 对象，用于异步操作
    AsioContext& io_context_;

};

#endif