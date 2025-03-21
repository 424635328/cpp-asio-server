#include "asio_context.h"
#include "server.h"
#include "http_session.h"
#include "http_request_handler.h"
#include <iostream>
#include <memory>
#include <boost/asio.hpp>

class MyHttpServer : public Server {
public:
    MyHttpServer(AsioContext& io_context, short port) : Server(io_context, port), request_handler_() {}

protected:
    std::shared_ptr<HttpSession> create_session(boost::asio::ip::tcp::socket socket) override {
        return std::make_shared<HttpSession>(std::move(socket), request_handler_);
    }

private:
    HttpRequestHandler request_handler_;
};

int main() {
    try {
        //AsioContext io_context; // 使用动态分配
        std::unique_ptr<AsioContext> io_context = std::make_unique<AsioContext>();
        MyHttpServer server(*io_context, 8088); // Listen on port 8088
        io_context->run(); // 使用指针

        // 添加循环，等待用户输入来停止服务器
        std::cout << "Press Enter to stop the server." << std::endl;
        std::cin.get(); // 等待用户输入

        std::cout << "Stopping the AsioContext." << std::endl;
        io_context->stop(); // 手动停止 AsioContext

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}