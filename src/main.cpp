// main.cpp
#include "asio_context.h"
#include "server.h"
#include "http_session.h"
#include "http_request_handler.h"
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <thread>
#include <chrono>
#include <boost/program_options.hpp>
#include <boost/system/error_code.hpp>
#include <atomic>  // 引入 atomic
#include <mutex>
#include <limits>

using namespace std;
using namespace boost::asio;
namespace po = boost::program_options;

// 自定义 HTTP 服务器类
class MyHttpServer : public Server {
public:
    // 构造函数
    MyHttpServer(AsioContext& io_context, short port, size_t max_connections)
        : Server(io_context.get_io_context(), port), request_handler_(),
          max_connections_(max_connections), current_connections_(0) {}

protected:
    // 创建会话（重写基类方法）
    std::shared_ptr<HttpSession> create_session(ip::tcp::socket socket) override {  // 修改返回类型
        std::lock_guard<std::mutex> lock(connection_mutex_);
        if (current_connections_ >= max_connections_) {
          std::cerr << "Reached the maximum connection limit, refused the connection." << std::endl;
            boost::system::error_code ec;
            socket.shutdown(ip::tcp::socket::shutdown_both, ec);
            socket.close(ec);
            return nullptr; // 拒绝连接
        }
        current_connections_++;
        return make_shared<HttpSession>(move(socket), request_handler_, [this](){ connection_finished(); }); // 传入回调函数
    }

private:
    HttpRequestHandler request_handler_;
    size_t max_connections_;
    std::atomic<size_t> current_connections_; // 使用 atomic 确保线程安全
    std::mutex connection_mutex_;

    void connection_finished() {
        std::lock_guard<std::mutex> lock(connection_mutex_);
        current_connections_--;
    }
};

int main(int argc, char* argv[]) {
    try {
        // 定义命令行选项
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message") // 帮助信息
            ("port", po::value<short>()->default_value(8765), "set listening port") // 设置监听端口
            ("max_connections", po::value<size_t>()->default_value(std::numeric_limits<size_t>::max()), "set max connections"); // 设置最大连接数

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm); // 解析命令行参数
        po::notify(vm);

        if (vm.count("help")) {
            cout << desc << endl;
            return 0;
        }

        short port = vm["port"].as<short>(); // 获取端口号
        size_t max_connections = vm["max_connections"].as<size_t>(); // 获取最大连接数
        bool port_found = false;

        for (int attempt = 0; attempt < 100; ++attempt)  // 尝试一系列端口
        {
            try
            {
                size_t num_threads = thread::hardware_concurrency(); // 获取硬件支持的线程数
                cout << "硬件支持的线程数: " << num_threads << endl;
                unique_ptr<AsioContext> io_context = make_unique<AsioContext>(num_threads);

                MyHttpServer server(*io_context, port, max_connections);

                std::thread io_thread([&]() {
                    io_context->run(); // 运行 AsioContext
                });

                std::this_thread::sleep_for(std::chrono::milliseconds(500));  // 添加一个小的延迟
                cout << "Server listening at http://127.0.0.1:" << port << " with max connections: " << max_connections << endl;
                port_found = true;


                // 等待用户输入停止服务器
                cout << "按下回车键停止服务器." << endl;
                cin.get(); // 等待输入

                // 停止 AsioContext
                cout << "正在停止 AsioContext." << endl;
                io_context->stop(); // 手动停止

                // Wait for the io_thread to finish
                io_thread.join(); // 等待 IO 线程结束
                break; // 成功启动服务器后退出循环


            }
            catch (const boost::system::system_error& ex)
            {
                if (ex.code() == boost::system::errc::address_in_use)
                {
                    cout << "Port " << port << " is in use. Trying the next port." << endl;
                    port++; // 递增以尝试下一个端口
                }
                else
                {
                    cerr << "异常: " << ex.what() << endl;
                    return 1;
                }
            }
            catch (exception& e) {
                cerr << "异常: " << e.what() << endl;
                return 1;
            }
        }

        if (!port_found)
        {
            cerr << "Could not find a free port after multiple attempts." << endl;
            return 1;
        }

    } catch (exception& e) {
        cerr << "异常: " << e.what() << endl;
        return 1;
    }

    return 0;
}