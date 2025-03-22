// asio_context.h
#ifndef ASIO_CONTEXT_H
#define ASIO_CONTEXT_H

#include <boost/asio.hpp>
#include <thread>
#include <vector>
#include <mutex> 
class AsioContext {
public:
    AsioContext(size_t thread_count);
    ~AsioContext();

    boost::asio::io_context& get_io_context();
    void run();
    void stop();

    static std::mutex cout_mutex; 

private:
    boost::asio::io_context io_context_;
    std::vector<std::thread> threads_;
    bool running_;
};

#endif