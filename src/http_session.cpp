#include "http_session.h"
#include "http_request_handler.h"
#include "http_request.h"
#include "http_response.h"
#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>
#include <vector> // Include vector for storing data

using namespace std;

HttpSession::HttpSession(boost::asio::ip::tcp::socket socket, HttpRequestHandler& request_handler)
    : socket_(std::move(socket)), request_handler_(request_handler)
{
    cout << "HTTP Session created." << endl;
}

HttpSession::~HttpSession() {
    cout << "HTTP Session destroyed." << endl;
}

void HttpSession::start() {
    cout << "HTTP Session started." << endl;
    try {
        do_read();
    } catch (std::exception& e) {
        cerr << "Exception in HttpSession::start: " << e.what() << endl;
        stop();
    }
}

void HttpSession::stop() {
    boost::system::error_code ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    if (ec) {
        cerr << "Error shutting down socket: " << ec.message() << endl;
    }
    socket_.close(ec);

    if (ec) {
        cerr << "Error closing socket: " << ec.message() << endl;
    }
    cout << "HTTP Session stopped." << endl;
}

void HttpSession::do_read() {
    auto self = shared_from_this();
    cout << "HttpSession::do_read - Starting async_read_until" << endl;
    boost::asio::async_read_until(socket_, buffer_, "\r\n\r\n",
        [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
            cout << "HttpSession::do_read - async_read_until callback - Started" << endl;
            cout << "HttpSession::do_read - async_read_until callback - Error code: " << ec.message() << endl;
            cout << "HttpSession::do_read - async_read_until callback - Bytes transferred: " << bytes_transferred << endl;
            try {
                if (!ec) {
                    cout << "HttpSession::do_read - async_read_until callback - Received " << bytes_transferred << " bytes." << endl;
                    istream request_stream(&buffer_);
                    HttpRequest request;
                    request_stream >> request;  //  从输入流中解析请求

                    handle_request(request); // 处理请求

                } else if (ec != boost::asio::error::eof) {
                    cerr << "HttpSession::do_read - async_read_until callback - Error reading: " << ec.message() << endl;
                    stop();
                } else {
                    cout << "HttpSession::do_read - async_read_until callback - Connection closed by client." << endl;
                    stop(); // Connection closed by client 关闭连接
                }
            } catch (std::exception& e) {
                cerr << "HttpSession::do_read - async_read_until callback - Exception: " << e.what() << endl;
                stop();
            }
            cout << "HttpSession::do_read - async_read_until callback - Finished" << endl;
        });
    cout << "HttpSession::do_read - async_read_until called" << endl;
}

void HttpSession::do_write(const HttpResponse& response) {
    auto self = shared_from_this();

    // Prepare the headers as a string
    stringstream header_stream;
    header_stream << response.version << " " << response.status_code << " OK\r\n";

    HttpResponse mutableResponse = response;
    mutableResponse.headers["Content-Length"] = to_string(response.body.size());

    for (const auto& header : mutableResponse.headers) {
        header_stream << header.first << ": " << header.second << "\r\n";
    }
    header_stream << "\r\n";
    string header_str = header_stream.str();

    cout << "HttpSession::do_write - Sending headers: " << header_str << endl;

    // Prepare the buffers for header and body
    vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(header_str));
    buffers.push_back(boost::asio::buffer(response.body)); // Body as string

    boost::asio::async_write(socket_, buffers,
        [this, self](boost::system::error_code ec, size_t bytes_transferred) {
            cout << "HttpSession::do_write - async_write callback - Started" << endl;
            cout << "HttpSession::do_write - async_write callback - Error code: " << ec.message() << endl;
            cout << "HttpSession::do_write - async_write callback - Bytes transferred: " << bytes_transferred << endl;

            try {
                if (!ec) {
                    cout << "HttpSession::do_write - async_write callback - Sent " << bytes_transferred << " bytes." << endl;
                    do_read(); // For persistent connections, read next request
                } else {
                    cerr << "HttpSession::do_write - async_write callback - Error writing: " << ec.message() << endl;
                    stop();
                }
            } catch (const std::exception& e) {
                cerr << "HttpSession::do_write - async_write callback - Exception: " << e.what() << endl;
                stop();
            }

            cout << "HttpSession::do_write - async_write callback - Finished" << endl;
        });
}

void HttpSession::handle_request(const HttpRequest& request) {
    cout << "HttpSession::handle_request - Handling request." << endl;
    try {
        HttpResponse response = request_handler_.handle_request(request);
        do_write(response);
    } catch (std::exception& e) {
        cerr << "HttpSession::handle_request - Exception: " << e.what() << endl;
        stop();
    }
}