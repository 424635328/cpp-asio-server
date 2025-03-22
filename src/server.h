// server.h
#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <memory>
#include "asio_context.h" 

class AsioContext;
class HttpSession;

class Server {
public:
    Server(AsioContext& io_context, short port);
    virtual ~Server();

protected:
    virtual std::shared_ptr<HttpSession> create_session(boost::asio::ip::tcp::socket socket) = 0;

private:
    void do_accept();

    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
};

#endif