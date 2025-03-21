#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <memory>

class AsioContext;
class HttpSession;  // Forward declaration

class Server {
public:
    Server(AsioContext& io_context, short port);
    virtual ~Server();

protected:
    virtual void do_accept(); // Implementation handles accepting connections
    virtual std::shared_ptr<HttpSession> create_session(boost::asio::ip::tcp::socket socket) = 0;
    boost::asio::ip::tcp::acceptor acceptor_;
    AsioContext& io_context_;

};

#endif