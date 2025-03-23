// http_session.h
#ifndef HTTP_SESSION_H
#define HTTP_SESSION_H

#include <boost/asio.hpp>
#include <memory>
#include "http_request.h"
#include "http_response.h"
#include <functional> 
class HttpRequestHandler;

class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
    HttpSession(boost::asio::ip::tcp::socket socket, HttpRequestHandler& request_handler, std::function<void()> connection_finished_callback);
    ~HttpSession();

    void start();
    void stop();

private:
    void do_read();
    void do_write(std::shared_ptr<HttpResponse> response);
    void handle_request(const HttpRequest& request);

    boost::asio::ip::tcp::socket socket_;
    HttpRequestHandler& request_handler_;
    boost::asio::streambuf buffer_;
    bool stopped_ = false;
    std::function<void()> connection_finished_callback_;
};

#endif