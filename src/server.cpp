#include "server.h"
#include "asio_context.h"
#include "http_session.h" // Include HttpSession.h
#include <iostream>

Server::Server(AsioContext& io_context, short port)
    : acceptor_(io_context.get_io_context(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
      io_context_(io_context) {
    std::cout << "Server::Server - Creating server on port " << port << std::endl;
    do_accept();
    std::cout << "Server listening on port " << port << std::endl;
}

Server::~Server() {
    std::cout << "Server::~Server - Shutting down server." << std::endl;
    std::cout << "Server::~Server - All threads joined." << std::endl;
}

void Server::do_accept() {
    std::cout << "Server::do_accept - Starting accept loop." << std::endl;
    acceptor_.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
            std::cout << "Server::do_accept - async_accept callback - Started" << std::endl;
            if (!ec) {
                std::cout << "Server::do_accept - New connection accepted." << std::endl;
                std::cout << "Server::do_accept - Socket local endpoint: " << socket.local_endpoint() << std::endl;
                std::cout << "Server::do_accept - Socket remote endpoint: " << socket.remote_endpoint() << std::endl;
                auto session = create_session(std::move(socket));
                if (session) {
                    std::cout << "Server::do_accept - Session created, starting session." << std::endl;
                    session->start();
                } else {
                    std::cerr << "Server::do_accept - Failed to create session." << std::endl;
                }
            } else {
                std::cerr << "Server::do_accept - Accept error: " << ec.message() << std::endl;
                std::cerr << "Server::do_accept - Accept error category: " << ec.category().name() << std::endl;
                std::cerr << "Server::do_accept - Accept error value: " << ec.value() << std::endl;
            }
            std::cout << "Server::do_accept - async_accept callback - Finished" << std::endl;
            do_accept(); // 再次调用 do_accept
        });
    std::cout << "Server::do_accept - async_accept called" << std::endl;
}