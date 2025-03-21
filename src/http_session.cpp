#include "http_session.h"
#include "http_request_handler.h"
#include "http_request.h"
#include "http_response.h"
#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

HttpSession::HttpSession(boost::asio::ip::tcp::socket socket, HttpRequestHandler& request_handler)
    : socket_(std::move(socket)), request_handler_(request_handler), buffer_(8192)  // 增加缓冲区大小
{
    std::cout << "HTTP Session created." << std::endl;
}

HttpSession::~HttpSession() {
    std::cout << "HTTP Session destroyed." << std::endl;
}

void HttpSession::start() {
    std::cout << "HTTP Session started." << std::endl;
    try {
        do_read();
    } catch (std::exception& e) {
        std::cerr << "Exception in HttpSession::start: " << e.what() << std::endl;
        stop();
    }
}

void HttpSession::stop() {
    boost::system::error_code ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec); // Use shutdown_both
    if (ec) {
        std::cerr << "Error shutting down socket: " << ec.message() << std::endl;
    }
    socket_.close(ec);

    if (ec) {
        std::cerr << "Error closing socket: " << ec.message() << std::endl;
    }
    std::cout << "HTTP Session stopped." << std::endl;
}

void HttpSession::do_read() {
    auto self = shared_from_this();
    std::cout << "HttpSession::do_read - Starting async_read_until" << std::endl;
    boost::asio::async_read_until(socket_, buffer_, "\r\n\r\n",
        [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
            std::cout << "HttpSession::do_read - async_read_until callback - Started" << std::endl;
            std::cout << "HttpSession::do_read - async_read_until callback - Error code: " << ec.message() << std::endl;
            std::cout << "HttpSession::do_read - async_read_until callback - Bytes transferred: " << bytes_transferred << std::endl;
            try {
                if (!ec) {
                    std::cout << "HttpSession::do_read - async_read_until callback - Received " << bytes_transferred << " bytes." << std::endl;
                    std::istream request_stream(&buffer_);
                    HttpRequest request;
                    request_stream >> request;  //  Parse the request

                    handle_request(request);

                } else if (ec != boost::asio::error::eof) {
                    std::cerr << "HttpSession::do_read - async_read_until callback - Error reading: " << ec.message() << std::endl;
                    stop();
                } else {
                    std::cout << "HttpSession::do_read - async_read_until callback - Connection closed by client." << std::endl;
                    stop(); // Connection closed by client
                }
            } catch (std::exception& e) {
                std::cerr << "HttpSession::do_read - async_read_until callback - Exception: " << e.what() << std::endl;
                stop();
            }
             std::cout << "HttpSession::do_read - async_read_until callback - Finished" << std::endl;
        });
     std::cout << "HttpSession::do_read - async_read_until called" << std::endl;
}

void HttpSession::do_write(const HttpResponse& response) {
    auto self = shared_from_this();
    std::string response_str = response.to_string(); // Convert the response object to a string
    std::cout << "HttpSession::do_write - Sending response: " << response_str << std::endl;

    boost::asio::async_write(socket_, boost::asio::buffer(response_str),
        [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
            std::cout << "HttpSession::do_write - async_write callback - Started" << std::endl;
            std::cout << "HttpSession::do_write - async_write callback - Error code: " << ec.message() << std::endl;
            std::cout << "HttpSession::do_write - async_write callback - Bytes transferred: " << bytes_transferred << std::endl;

            try {
                if (!ec) {
                    std::cout << "HttpSession::do_write - async_write callback - Sent " << bytes_transferred << " bytes." << std::endl;
                    //Initiate the next read for the next request.  For persistent connections.
                    do_read();
                } else {
                    std::cerr << "HttpSession::do_write - async_write callback - Error writing: " << ec.message() << std::endl;
                    stop();
                }
            } catch (std::exception& e) {
                std::cerr << "HttpSession::do_write - async_write callback - Exception: " << e.what() << std::endl;
                stop();
            }
            std::cout << "HttpSession::do_write - async_write callback - Finished" << std::endl;
        });
}

void HttpSession::handle_request(const HttpRequest& request) {
    std::cout << "HttpSession::handle_request - Handling request." << std::endl;
    try {
        HttpResponse response = request_handler_.handle_request(request);
        do_write(response);
    } catch (std::exception& e) {
        std::cerr << "HttpSession::handle_request - Exception: " << e.what() << std::endl;
        stop();
    }
}