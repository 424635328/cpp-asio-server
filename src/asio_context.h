#ifndef ASIO_CONTEXT_H
#define ASIO_CONTEXT_H

#include <boost/asio.hpp>
#include <thread>
#include <vector>

using namespace std;

// AsioContext 类，管理 io_context 和工作线程
class AsioContext {
public:
    // 构造函数，创建指定数量的线程，默认使用硬件支持的最大线程数
    AsioContext(size_t thread_count = thread::hardware_concurrency());
    
    // 析构函数，确保资源释放
    ~AsioContext();

    // 获取 io_context 引用，供外部使用
    boost::asio::io_context& get_io_context();
    
    // 运行 io_context
    void run();
    
    // 停止 io_context，并等待所有线程结束
    void stop();

private:
    boost::asio::io_context io_context_; // Boost Asio 上下文对象
    vector<thread> threads_; // 线程池
    bool running_ = false; // 运行状态标志
};

#endif