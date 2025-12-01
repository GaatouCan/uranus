#include "MultiIOContextPool.h"

namespace uranus {
    MultiIOContextPool::PoolNode::PoolNode()
        : guard(asio::make_work_guard(ctx)) {
    }

    MultiIOContextPool::MultiIOContextPool()
        : next_(0) {
    }

    MultiIOContextPool::~MultiIOContextPool() {
        stop();

        for (auto &[th, ctx, guard]: pool_) {
            if (th.joinable()) {
                th.join();
            }
        }
    }

    void MultiIOContextPool::start(const size_t capacity) {
        pool_ = std::vector<PoolNode>(capacity);
        for (auto &[th, ctx, guard]: pool_) {
            th = std::thread([&ctx] {
                ctx.run();
            });
        }
    }

    void MultiIOContextPool::stop() {
        for (auto &[th, ctx, guard]: pool_) {
            if (!ctx.stopped()) {
                guard.reset();
                ctx.stop();
            }
        }
    }

    asio::io_context &MultiIOContextPool::getIOContext() {
        if (pool_.empty()) {
            throw std::runtime_error("MultiIOContextPool::getIOContext(): pool is empty");
        }

        const size_t idx = next_.fetch_add(1, std::memory_order_relaxed);
        return pool_[idx % pool_.size()].ctx;
    }
}
