#pragma once

#include "base.export.h"
#include "noncopy.h"

#include <asio/io_context.hpp>
#include <thread>
#include <vector>


namespace uranus {
    class BASE_API SingleIOContextPool final {

    public:
        SingleIOContextPool();
        ~SingleIOContextPool();

        DISABLE_COPY_MOVE(SingleIOContextPool)

        void start(size_t capacity = 4);
        void stop();

        [[nodiscard]] asio::io_context& getIOContext();

    private:
        asio::io_context ctx_;
        asio::executor_work_guard<asio::io_context::executor_type> guard_;
        std::vector<std::thread> pool_;
    };
}