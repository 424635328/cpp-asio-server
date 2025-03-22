#include "http_session.h"
#include "http_request_handler.h"
#include "http_request.h"
#include "http_response.h"
#include "asio_context.h"
#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>
#include <vector>
#include <memory>

using namespace std;

HttpSession::HttpSession(boost::asio::ip::tcp::socket socket, HttpRequestHandler& request_handler)
    : socket_(std::move(socket)), request_handler_(request_handler)
{
    {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
        cout << "HTTP Session created." << endl;
    }
}

HttpSession::~HttpSession() {
    {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
        cout << "HTTP Session destroyed." << endl;
    }
}

void HttpSession::start() {
    {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
        cout << "HTTP Session started." << endl;
    }
    try {
        do_read();
    } catch (std::exception& e) {
        {
            std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
            cerr << "Exception in HttpSession::start: " << e.what() << endl;
        }
        stop();
    }
}

void HttpSession::stop() {
    boost::system::error_code ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    if (ec) {
        {
            std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
            cerr << "Error shutting down socket: " << ec.message() << endl;
        }
    }
    socket_.close(ec);

    if (ec) {
        {
            std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
            cerr << "Error closing socket: " << ec.message() << endl;
        }
    }
    {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
        cout << "HTTP Session stopped." << endl;
    }
}

void HttpSession::do_read() {
    auto self = shared_from_this();
    {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
        cout << "HttpSession::do_read - Starting async_read_until" << endl;
        cout << "HttpSession::do_read - async_read_until called" << endl;
    }
    boost::asio::async_read_until(socket_, buffer_, "\r\n\r\n",
        [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
            {
                std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                cout << "HttpSession::do_read - async_read_until callback - Started" << endl;
                cout << "HttpSession::do_read - async_read_until callback - Error code: " << ec.message() << endl;
                cout << "HttpSession::do_read - async_read_until callback - Bytes transferred: " << bytes_transferred << endl;
            }
            try {
                if (!ec) {
                    if (bytes_transferred == 0) {
                        {
                            std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                            cout << "HttpSession::do_read - async_read_until callback - Received 0 bytes. Closing connection." << endl;
                        }
                        stop(); // 关闭连接
                        return;
                    }
                    {
                        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                        cout << "HttpSession::do_read - async_read_until callback - Received " << bytes_transferred << " bytes." << endl;
                    }
                    istream request_stream(&buffer_);
                    HttpRequest request;
                    request_stream >> request;  //  从输入流中解析请求

                    handle_request(request); // 处理请求

                } else if (ec != boost::asio::error::eof) {
                    {
                        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                        cerr << "HttpSession::do_read - async_read_until callback - Error reading: " << ec.message() << endl;
                    }
                    stop();
                } else {
                    {
                        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                        cout << "HttpSession::do_read - async_read_until callback - Connection closed by client." << endl;
                    }
                    stop(); // Connection closed by client 关闭连接
                }
            } catch (std::exception& e) {
                {
                    std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                    cerr << "HttpSession::do_read - async_read_until callback - Exception: " << e.what() << endl;
                }
                stop();
            }
            {
                std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                cout << "HttpSession::do_read - async_read_until callback - Finished" << endl;
            }
        });
}

void HttpSession::do_write(std::shared_ptr<HttpResponse> response) {  
    auto self = shared_from_this();

    stringstream header_stream;
    header_stream << response->version << " " << response->status_code << " OK\r\n";

    HttpResponse mutableResponse = *response; 
    mutableResponse.headers["Content-Length"] = to_string(response->body.size());

    for (const auto& header : mutableResponse.headers) {
        header_stream << header.first << ": " << header.second << "\r\n";
    }
    header_stream << "\r\n";
    string header_str = header_stream.str();

    {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
        cout << "HttpSession::do_write - Sending headers: " << header_str << endl;
    }

    // Prepare the buffers for header and body
    vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(header_str));
    buffers.push_back(boost::asio::buffer(response->body)); 

    boost::asio::async_write(socket_, buffers,
        [this, self, response](boost::system::error_code ec, size_t bytes_transferred) {

            {
                std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                cout << "HttpSession::do_write - async_write callback - Started" << endl;
                cout << "HttpSession::do_write - async_write callback - Error code: " << ec.message() << endl;
                cout << "HttpSession::do_write - async_write callback - Bytes transferred: " << bytes_transferred << endl;
            }

            try {
                if (!ec) {
                    {
                        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                        cout << "HttpSession::do_write - async_write callback - Sent " << bytes_transferred << " bytes." << endl;
                    }
                    do_read(); // For persistent connections, read next request
                } else {
                    {
                        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                        cerr << "HttpSession::do_write - async_write callback - Error writing: " << ec.message() << endl;
                    }
                    stop();
                }
            } catch (const std::exception& e) {
                {
                    std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                    cerr << "HttpSession::do_write - async_write callback - Exception: " << e.what() << endl;
                }
                stop();
            }

            {
                std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
                cout << "HttpSession::do_write - async_write callback - Finished" << endl;
            }
        });
}

void HttpSession::handle_request(const HttpRequest& request) {
    {
        std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
        cout << "HttpSession::handle_request - Handling request." << endl;
    }
    try {
        // Create a shared_ptr for the HttpResponse
        std::shared_ptr<HttpResponse> response = std::make_shared<HttpResponse>(request_handler_.handle_request(request));
        do_write(response);
    } catch (std::exception& e) {
        {
            std::lock_guard<std::mutex> lock(AsioContext::cout_mutex);
            cerr << "HttpSession::handle_request - Exception: " << e.what() << endl;
        }
        stop();
    }
}