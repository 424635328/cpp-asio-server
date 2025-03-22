#include "asio_context.h"
#include "server.h"
#include "http_session.h"
#include "http_request_handler.h"
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <thread> // Include thread header

using namespace std;
using namespace boost::asio;

// 自定义 HTTP 服务器类
class MyHttpServer : public Server {
public:
    // 构造函数
    MyHttpServer(AsioContext& io_context, short port) : Server(io_context, port), request_handler_() {}

protected:
    // 创建会话（重写基类方法）
    shared_ptr<HttpSession> create_session(ip::tcp::socket socket) override {
        return make_shared<HttpSession>(move(socket), request_handler_);
    }

private:
    // 请求处理器
    HttpRequestHandler request_handler_;
};

int main() {
    try {
        size_t num_threads = thread::hardware_concurrency(); // 获取硬件支持的线程数
        cout << "硬件支持的线程数: " << num_threads << endl;
        // 创建 AsioContext 对象（使用智能指针）
        unique_ptr<AsioContext> io_context = make_unique<AsioContext>(num_threads);
        // 创建 MyHttpServer 对象
        MyHttpServer server(*io_context, 8765);

        // 运行 AsioContext in a separate thread
        std::thread io_thread([&]() {
            io_context->run();
        });

        // Wait for a short time to ensure threads are running before printing the message
        std::this_thread::sleep_for(std::chrono::milliseconds(500));  // Add a small delay

        // 等待用户输入停止服务器
        cout << "按下回车键停止服务器." << endl; // Move here
        cin.get(); // 等待输入

        // 停止 AsioContext
        cout << "正在停止 AsioContext." << endl;
        io_context->stop(); // 手动停止

        // Wait for the io_thread to finish
        io_thread.join();


    } catch (exception& e) {
        cerr << "异常: " << e.what() << endl;
    }

    return 0;
}