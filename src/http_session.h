#ifndef HTTP_SESSION_H
#define HTTP_SESSION_H

#include <memory>
#include <boost/asio.hpp>
#include "http_request_handler.h"
#include "http_request.h"
#include "http_response.h"

class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
    HttpSession(boost::asio::ip::tcp::socket socket, HttpRequestHandler& request_handler);
    ~HttpSession();

    void start();
    void stop();

private:
    void do_read();
    void do_write(const HttpResponse& response);
    void handle_request(const HttpRequest& request);

    boost::asio::ip::tcp::socket socket_;
    HttpRequestHandler& request_handler_;
    boost::asio::streambuf buffer_;
    const size_t buffer_size = 8192;  // 缓冲区大小
};

#endif