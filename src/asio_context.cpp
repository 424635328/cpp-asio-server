#include "asio_context.h"
#include <iostream>

using namespace std;
using namespace boost::asio; 

// AsioContext 类，管理 Asio 的 io_context 和线程
AsioContext::AsioContext(size_t thread_count) : threads_(thread_count) {
    cout << "创建 AsioContext，线程数: " << thread_count << endl;
    running_ = true; // 设置运行状态为 true
    // 创建线程池
    for (size_t i = 0; i < thread_count; ++i) {
        threads_[i] = thread([this, i]() {  // 传递 'i' 的值
            try {
                cout << "AsioContext::AsioContext - 线程 " << i << " 启动" << endl;
                io_context_.run(); // 运行 io_context
                cout << "AsioContext::AsioContext - 线程 " << i << " 结束" << endl;
            } catch (const exception& e) {
                cerr << "io_context_.run() 异常: " << e.what() << endl;
            }
        });
    }
}

// 析构函数，停止 AsioContext
AsioContext::~AsioContext() {
    stop();
}

// 获取 io_context 对象
io_context& AsioContext::get_io_context() {
    return io_context_;
}

// 运行 io_context
void AsioContext::run() {
    if (!running_) {
        running_ = true; // 设置运行状态为 true
        try {
            cout << "AsioContext::run - io_context_.run() - 启动" << endl;
            io_context_.run(); // 运行 io_context
            cout << "AsioContext::run - io_context_.run() - 结束" << endl;
        } catch (const exception& e) {
            cerr << "io_context_.run() 异常: " << e.what() << endl;
        }
    }
}

// 停止 io_context
void AsioContext::stop() {
    if (running_) {
        running_ = false; // 设置运行状态为 false
        cout << "AsioContext::stop - 停止 io_context_." << endl;
        io_context_.stop(); // 停止 io_context
        cout << "AsioContext::stop - 加入线程." << endl;
        // 等待所有线程结束
        for (auto& thread : threads_) {
            if (thread.joinable()) {
                thread.join(); // 等待线程结束
                cout << "AsioContext::stop - Thread has been joined." << endl;
            }
        }
        cout << "AsioContext::stop - 所有线程已合并." << endl;
    }
}