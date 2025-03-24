// Description: 使用 Boost.Asio 实现的简单 HTTP 客户端，支持超时。
#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>
#include <sstream>
#include <string>
#include <vector>
#include <atomic>
#include <memory>

using namespace boost::asio;
using namespace boost::asio::ip;

int main() {
    try {
        // 1. 创建 I/O 上下文
        io_context io_context;

        // 2. 创建 TCP socket
        tcp::socket socket(io_context);

        // 3. 解析服务器地址和端口
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve("127.0.0.1", "8765"); // 替换为你的服务器地址和端口

        // 4. 连接到服务器
        boost::asio::connect(socket, endpoints);

        // 5. 构建 HTTP 请求
        std::string request = "GET / HTTP/1.1\r\n"
                              "Host: 127.0.0.1:8765\r\n"
                              "User-Agent: SimpleBoostAsioClient\r\n"
                              "Connection: close\r\n\r\n";

        // 6. 发送请求
        boost::asio::write(socket, boost::asio::buffer(request));
        std::cout << "Sent HTTP request:\n" << request << std::endl;

        // 7. 读取响应头部
        boost::asio::streambuf response_buffer;
        boost::asio::read_until(socket, response_buffer, "\r\n\r\n");

        // 8. 解析响应头部，提取 Content-Length
        std::istream header_stream(&response_buffer);
        std::string header_line;
        size_t content_length = 0;
        while (std::getline(header_stream, header_line) && header_line != "\r") {
            std::cout << "Response Header: " << header_line << std::endl;
            if (header_line.find("Content-Length:") != std::string::npos) {
                std::stringstream ss(header_line.substr(header_line.find(":") + 1));
                ss >> content_length;
                std::cout << "Content-Length: " << content_length << std::endl; // 调试信息
            }
        }

        // 9. 读取响应体 (带超时)
        std::shared_ptr<std::vector<char>> body_data = std::make_shared<std::vector<char>>(content_length);
        size_t total_bytes_read = 0;
        boost::system::error_code ec;
        std::atomic<bool> timed_out{false};
        std::shared_ptr<boost::asio::steady_timer> timeout = std::make_shared<boost::asio::steady_timer>(io_context, std::chrono::seconds(30));
        std::shared_ptr<tcp::socket> shared_socket = std::make_shared<tcp::socket>(std::move(socket));

        auto read_handler = [&](const boost::system::error_code& error, size_t bytes_transferred) {
            ec = error;
            std::cout << "bytes_transferred: " << bytes_transferred << std::endl;

            if (error) {
                std::cerr << "async_read_some error: " << error.message() << std::endl;
                if (error == boost::asio::error::eof) {
                    std::cerr << "Connection closed prematurely by the server." << std::endl;
                }
                timeout->cancel();
            } else {
                total_bytes_read += bytes_transferred;
                std::cout << "Total bytes read: " << total_bytes_read << std::endl;
            }
            io_context.stop(); // 停止 io_context
        };

        auto timeout_handler = [&](const boost::system::error_code& error) {
            if (!error) {
                std::cerr << "Read timeout!" << std::endl;
                timed_out = true;
                shared_socket->close(); // 关闭 socket，取消读取操作
            } else if (error == boost::asio::error::operation_aborted) {
                std::cout << "Timeout timer cancelled." << std::endl;
            } else {
                std::cerr << "Timeout timer error: " << error.message() << std::endl;
            }
            io_context.stop(); // 停止 io_context
        };

        std::function<void()> start_read = [&]() {
            if (total_bytes_read < content_length && !timed_out) {
                timeout->expires_after(std::chrono::seconds(30));
                timeout->async_wait(timeout_handler);

                shared_socket->async_read_some(boost::asio::buffer(body_data->data() + total_bytes_read, content_length - total_bytes_read), read_handler);

                std::cout << "Starting io_context.run()..." << std::endl;
                io_context.run();
                io_context.restart();

                std::cout << "io_context.run() finished." << std::endl;
            }
        };

        start_read(); // 启动首次读取

        if (timed_out) {
            std::cerr << "Read operation timed out." << std::endl;
        } else if (ec && ec != boost::asio::error::eof) {
            throw boost::system::system_error(ec); // 其他错误
        }

        // 使用实际读取的字节数创建字符串
        std::string body(body_data->data(), total_bytes_read);

        std::cout << "\nResponse Body (" << total_bytes_read << " bytes):\n" << body << std::endl;

        // 10. 关闭 socket
        shared_socket->close();

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}