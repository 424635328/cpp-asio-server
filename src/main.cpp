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

using namespace std;
using namespace boost::asio;
namespace po = boost::program_options;

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

int main(int argc, char* argv[]) {
    try {
        // 定义命令行选项
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("port", po::value<short>()->default_value(8765), "set listening port");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            cout << desc << endl;
            return 0;
        }

        short port = vm["port"].as<short>();
        bool port_found = false;

        for (int attempt = 0; attempt < 100; ++attempt)  // Try a range of ports
        {
            try
            {
                size_t num_threads = thread::hardware_concurrency(); // 获取硬件支持的线程数
                cout << "硬件支持的线程数: " << num_threads << endl;
                // 创建 AsioContext 对象（使用智能指针）
                unique_ptr<AsioContext> io_context = make_unique<AsioContext>(num_threads);

                // 创建 MyHttpServer 对象
                MyHttpServer server(*io_context, port);

                // 运行 AsioContext in a separate thread
                std::thread io_thread([&]() {
                    io_context->run();
                });

                // Wait for a short time to ensure threads are running before printing the message
                std::this_thread::sleep_for(std::chrono::milliseconds(500));  // Add a small delay
                cout << "Server listening at http://127.0.0.1:" << port << endl;
                port_found = true;


                // 等待用户输入停止服务器
                cout << "按下回车键停止服务器." << endl;
                cin.get(); // 等待输入

                // 停止 AsioContext
                cout << "正在停止 AsioContext." << endl;
                io_context->stop(); // 手动停止

                // Wait for the io_thread to finish
                io_thread.join();
                break; // Exit the loop as we've successfully started the server


            }
            catch (const boost::system::system_error& ex)
            {
                if (ex.code() == boost::system::errc::address_in_use)
                {
                    cout << "Port " << port << " is in use. Trying the next port." << endl;
                    port++; // Increment to try the next port.
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