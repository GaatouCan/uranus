#include "SingleIOContextPool.h"

namespace uranus {
    SingleIOContextPool::SingleIOContextPool()
        : guard_(asio::make_work_guard(ctx_)) {
    }

    SingleIOContextPool::~SingleIOContextPool() {
        stop();

        for (auto &val: pool_) {
            if (val.joinable()) {
                val.join();
            }
        }
    }

    void SingleIOContextPool::start(size_t capacity) {
        pool_ = std::vector<std::thread>(capacity);
        for (auto &val: pool_) {
            val = std::thread([this]() {
                ctx_.run();
            });
        }
    }

    void SingleIOContextPool::stop() {
        if (ctx_.stopped())
            return;

        guard_.reset();
        ctx_.stop();
    }

    asio::io_context &SingleIOContextPool::getIOContext() {
        return ctx_;
    }
}
