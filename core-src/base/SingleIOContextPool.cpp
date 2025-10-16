#include "SingleIOContextPool.h"


SingleIOContextPool::SingleIOContextPool()
    : guard_(asio::make_work_guard(ctx_)) {

}

SingleIOContextPool::~SingleIOContextPool() {
    Stop();

    for (auto &val: list_) {
        if (val.joinable()) {
            val.join();
        }
    }
}

void SingleIOContextPool::Start(const size_t capacity) {
    list_ = std::vector<std::thread>(capacity);
    for (auto &val: list_) {
        val = std::thread([this] {
            ctx_.run();
        });
    }
}

void SingleIOContextPool::Stop() {
    if (ctx_.stopped())
        return;

    guard_.reset();
    ctx_.stop();
}

asio::io_context &SingleIOContextPool::GetIOContext() {
    return ctx_;
}
