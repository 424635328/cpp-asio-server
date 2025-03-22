// asio_context.cpp
#include "asio_context.h"
#include <iostream>

using namespace std;
using namespace boost::asio;

std::mutex AsioContext::cout_mutex; // Define the static mutex

AsioContext::AsioContext(size_t thread_count) : threads_(thread_count) {
    {
        std::lock_guard<std::mutex> lock(cout_mutex); // Get the lock
        cout << "创建 AsioContext，线程数: " << thread_count << endl;
    }
    running_ = true;
    for (size_t i = 0; i < thread_count; ++i) {
        threads_[i] = thread([this, i]() {
            try {
                {
                    std::lock_guard<std::mutex> lock(cout_mutex); // Get the lock
                    cout << "AsioContext::AsioContext - 线程 " << i << " 启动" << endl;
                }
                io_context_.run();
                {
                    std::lock_guard<std::mutex> lock(cout_mutex); // Get the lock
                    cout << "AsioContext::AsioContext - 线程 " << i << " 结束" << endl;
                }
            } catch (const exception& e) {
                {
                    std::lock_guard<std::mutex> lock(cout_mutex); // Get the lock
                    cerr << "io_context_.run() 异常: " << e.what() << endl;
                }
            }
        });
    }
}

AsioContext::~AsioContext() {
    stop();
}

io_context& AsioContext::get_io_context() {
    return io_context_;
}

void AsioContext::run() {
    if (!running_) {
        running_ = true;
        try {
            {
                std::lock_guard<std::mutex> lock(cout_mutex); // Get the lock
                cout << "AsioContext::run - io_context_.run() - 启动" << endl;
            }
            io_context_.run();
            {
                std::lock_guard<std::mutex> lock(cout_mutex); // Get the lock
                cout << "AsioContext::run - io_context_.run() - 结束" << endl;
            }
        } catch (const exception& e) {
            {
                std::lock_guard<std::mutex> lock(cout_mutex); // Get the lock
                cerr << "io_context_.run() 异常: " << e.what() << endl;
            }
        }
    }
}

void AsioContext::stop() {
    if (running_) {
        running_ = false;
        {
            std::lock_guard<std::mutex> lock(cout_mutex); // Get the lock
            cout << "AsioContext::stop - 停止 io_context_." << endl;
        }
        io_context_.stop();
        {
            std::lock_guard<std::mutex> lock(cout_mutex); // Get the lock
            cout << "AsioContext::stop - 加入线程." << endl;
        }
        for (auto& thread : threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        {
            std::lock_guard<std::mutex> lock(cout_mutex); // Get the lock
            cout << "AsioContext::stop - 所有线程已合并." << endl;
        }
    }
}