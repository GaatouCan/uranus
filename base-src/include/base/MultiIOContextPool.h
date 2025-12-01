#pragma once

#include "base.export.h"
#include "noncopy.h"

#include <asio/io_context.hpp>
#include <thread>
#include <vector>
#include <atomic>

namespace uranus {

    class BASE_API MultiIOContextPool final {

        struct PoolNode {
            std::thread th;
            asio::io_context ctx;
            asio::executor_work_guard<asio::io_context::executor_type> guard;

            PoolNode();
        };

    public:
        MultiIOContextPool();
        ~MultiIOContextPool();

        DISABLE_COPY_MOVE(MultiIOContextPool)

        void start(size_t capacity = 4);
        void stop();

        asio::io_context& getIOContext();

    private:
        std::vector<PoolNode> pool_;
        std::atomic<size_t> next_;
    };
}
