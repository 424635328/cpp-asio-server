#include "asio_context.h"
#include <iostream>
#include <boost/asio/post.hpp>

using namespace std;
using namespace boost::asio;

std::mutex AsioContext::cout_mutex; // 定义静态互斥锁

AsioContext::AsioContext(size_t thread_count) : threads_(thread_count) {
    std::lock_guard<std::mutex> lock(cout_mutex); // 获取锁
    cout << "创建 AsioContext，线程数: " << thread_count << endl;
    running_ = true;
    for (size_t i = 0; i < thread_count; ++i) {
        threads_[i] = thread([this, i]() {
            try {
                { // 获取锁，保证线程安全
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    cout << "AsioContext::AsioContext - 线程 " << i << " 启动" << endl;
                }
                io_context_.run();
                { // 获取锁，保证线程安全
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    cout << "AsioContext::AsioContext - 线程 " << i << " 结束" << endl;
                }
            } catch (const exception& e) {
                 { // 获取锁，保证线程安全
                    std::lock_guard<std::mutex> lock(cout_mutex);
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
         { // 获取锁，保证线程安全
            std::lock_guard<std::mutex> lock(cout_mutex);
            cout << "AsioContext::run - io_context_.run() - 启动" << endl;
         }
        running_ = true;
        try {
            io_context_.run();
            { // 获取锁，保证线程安全
                std::lock_guard<std::mutex> lock(cout_mutex);
                cout << "AsioContext::run - io_context_.run() - 结束" << endl;
            }
        } catch (const exception& e) {
            { // 获取锁，保证线程安全
                std::lock_guard<std::mutex> lock(cout_mutex);
                cerr << "io_context_.run() 异常: " << e.what() << endl;
            }
        }
    }
}

void AsioContext::stop() {
    if (running_) {
         { // 获取锁，保证线程安全
            std::lock_guard<std::mutex> lock(cout_mutex);
            cout << "AsioContext::stop - 停止 io_context_." << endl;
        }

        running_ = false;

        for (size_t i = 0; i < threads_.size(); ++i) {
            boost::asio::post(io_context_, [](){}); //  Post an empty task
        }

        io_context_.stop(); // Stop accepting new tasks
         { // 获取锁，保证线程安全
            std::lock_guard<std::mutex> lock(cout_mutex);
            cout << "AsioContext::stop - 加入线程." << endl;
        }
        for (auto& thread : threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
         { // 获取锁，保证线程安全
            std::lock_guard<std::mutex> lock(cout_mutex);
            cout << "AsioContext::stop - 所有线程已合并." << endl;
        }
    }
}

//deadlock resolution
// 不使用 std::lock_guard<std::mutex> lock(cout_mutex); 可能会导致死锁

// #include "asio_context.h"
// #include <iostream>
// #include <boost/asio/post.hpp>

// using namespace std;
// using namespace boost::asio;

// std::mutex AsioContext::cout_mutex; // 定义静态互斥锁

// AsioContext::AsioContext(size_t thread_count) : threads_(thread_count) {
//     std::lock_guard<std::mutex> lock(cout_mutex); // 获取锁
//     cout << "创建 AsioContext，线程数: " << thread_count << endl;
//     running_ = true;
//     for (size_t i = 0; i < thread_count; ++i) {
//         threads_[i] = thread([this, i]() {
//             try {
//                 cout << "AsioContext::AsioContext - 线程 " << i << " 启动" << endl;
//                 io_context_.run();
//                 cout << "AsioContext::AsioContext - 线程 " << i << " 结束" << endl;
//             } catch (const exception& e) {
//                 cerr << "io_context_.run() 异常: " << e.what() << endl;
//             }
//         });
//     }
// }

// AsioContext::~AsioContext() {
//     stop();
// }

// io_context& AsioContext::get_io_context() {
//     return io_context_;
// }

// void AsioContext::run() {
//     if (!running_) {
//         running_ = true;
//         try {
//             cout << "AsioContext::run - io_context_.run() - 启动" << endl;
//             io_context_.run();
//             cout << "AsioContext::run - io_context_.run() - 结束" << endl;
//         } catch (const exception& e) {
//             cerr << "io_context_.run() 异常: " << e.what() << endl;
//         }
//     }
// }

// void AsioContext::stop() {
//     if (running_) {
//         running_ = false;
//         cout << "AsioContext::stop - 停止 io_context_." << endl;

//         for (size_t i = 0; i < threads_.size(); ++i) {
//             boost::asio::post(io_context_, [](){}); //  Post an empty task
//         }

//         io_context_.stop(); // Stop accepting new tasks
//         cout << "AsioContext::stop - 加入线程." << endl;
//         for (auto& thread : threads_) {
//             if (thread.joinable()) {
//                 thread.join();
//             }
//         }
//         cout << "AsioContext::stop - 所有线程已合并." << endl;
//     }
// }