#pragma once

#include "../Common.h"

#include <asio/io_context.hpp>
#include <vector>
#include <atomic>
#include <thread>


using std::atomic;
using std::vector;


class CORE_API MultiIOContextPool final {

    struct PoolNode {

        std::thread thread;
        asio::io_context context;
        asio::executor_work_guard<asio::io_context::executor_type> guard;

        PoolNode();
    };

public:
    MultiIOContextPool();
    ~MultiIOContextPool();

    DISABLE_COPY_MOVE(MultiIOContextPool)

    void Start(size_t count);
    void Stop();

    asio::io_context& GetIOContext();

private:
    vector<PoolNode> list_;
    atomic<size_t> next_index_;
};
