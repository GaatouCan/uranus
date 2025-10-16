#pragma once

#include "../Common.h"

#include <asio/io_context.hpp>
#include <vector>
#include <thread>


class CORE_API SingleIOContextPool final {

public:
    SingleIOContextPool();
    ~SingleIOContextPool();

    DISABLE_COPY_MOVE(SingleIOContextPool);

    void Start(size_t capacity = 4);
    void Stop();

    [[nodiscard]] asio::io_context& GetIOContext();

private:
    asio::io_context ctx_;
    asio::executor_work_guard<asio::io_context::executor_type> guard_;
    std::vector<std::thread> list_;
};


