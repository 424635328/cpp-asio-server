// http_session.h
#ifndef HTTP_SESSION_H
#define HTTP_SESSION_H

#include <boost/asio.hpp>
#include <memory>
#include <mutex>
#include "http_request_handler.h" // Forward declaration
#include "http_request.h"
#include "http_response.h"

class HttpSession : public std::enable_shared_from_this<HttpSession> { // 继承 enable_shared_from_this
public:
    HttpSession(boost::asio::ip::tcp::socket socket, HttpRequestHandler& request_handler);
    ~HttpSession();

    void start();
    void stop();

private:
    void do_read();
    void do_write(std::shared_ptr<HttpResponse> response);
    void handle_request(const HttpRequest& request);

    boost::asio::ip::tcp::socket socket_;
    boost::asio::streambuf buffer_;
    HttpRequestHandler& request_handler_;
    bool stopped_ = false;
};

#endif