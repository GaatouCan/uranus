#include "MultiIOContextPool.h"

namespace uranus {
    MultiIOContextPool::PoolNode::PoolNode()
        : guard(asio::make_work_guard(context)) {
    }

    MultiIOContextPool::MultiIOContextPool()
        : next_index_(0) {
    }

    MultiIOContextPool::~MultiIOContextPool() {
        Stop();

        for (auto &node: list_) {
            if (node.thread.joinable()) {
                node.thread.join();
            }
        }
    }

    void MultiIOContextPool::Start(const size_t count) {
        list_ = std::vector<PoolNode>(count);
        for (auto &node: list_) {
            node.thread = std::thread([&node] {
                node.context.run();
            });
        }
    }

    void MultiIOContextPool::Stop() {
        for (auto &[th, ctx, guard]: list_) {
            if (!ctx.stopped()) {
                guard.reset();
                ctx.stop();
            }
        }
    }

    asio::io_context &MultiIOContextPool::GetIOContext() {
        if (list_.empty())
            throw std::runtime_error("No IOContext");

        const size_t index = next_index_.fetch_add(1, std::memory_order_relaxed);
        return list_[index % list_.size()].context;
    }
}
