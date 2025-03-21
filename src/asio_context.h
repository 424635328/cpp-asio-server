#ifndef ASIO_CONTEXT_H
#define ASIO_CONTEXT_H

#include <boost/asio.hpp>
#include <thread>
#include <vector>

class AsioContext {
public:
    AsioContext(size_t thread_count = std::thread::hardware_concurrency());
    ~AsioContext();

    boost::asio::io_context& get_io_context();
    void run();
    void stop();

private:
    boost::asio::io_context io_context_;
    std::vector<std::thread> threads_;
    bool running_ = false;
};

#endif