#include "asio_context.h"
#include <iostream>

AsioContext::AsioContext(size_t thread_count) : threads_(thread_count) {
    std::cout << "Creating AsioContext with " << thread_count << " threads." << std::endl;
    running_ = true;
    for (size_t i = 0; i < thread_count; ++i) {
        threads_[i] = std::thread([this, i]() {  // Pass 'i' by value
            try {
                std::cout << "AsioContext::AsioContext - Thread " << i << " started" << std::endl;
                io_context_.run();
                std::cout << "AsioContext::AsioContext - Thread " << i << " finished" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Exception in io_context_.run(): " << e.what() << std::endl;
            }
        });
    }
}

AsioContext::~AsioContext() {
    stop();
}

boost::asio::io_context& AsioContext::get_io_context() {
    return io_context_;
}

void AsioContext::run() {
    if (!running_) {
        running_ = true;
        try {
            std::cout << "AsioContext::run - io_context_.run() - Starting" << std::endl;
            io_context_.run();
            std::cout << "AsioContext::run - io_context_.run() - Finished" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Exception in io_context_.run(): " << e.what() << std::endl;
        }
    }
}

void AsioContext::stop() {
    if (running_) {
        running_ = false;
        std::cout << "AsioContext::stop - Stopping io_context_." << std::endl;
        io_context_.stop();
        std::cout << "AsioContext::stop - Joining threads." << std::endl;
        for (auto& thread : threads_) {
            if (thread.joinable()) {
                thread.join();
                std::cout << "AsioContext::stop - Thread joined." << std::endl;
            }
        }
        std::cout << "AsioContext::stop - All threads joined." << std::endl;
    }
}