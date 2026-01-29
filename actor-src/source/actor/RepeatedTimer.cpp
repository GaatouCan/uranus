#include "RepeatedTimer.h"
#include "BaseActorContext.h"

#include <asio.hpp>

namespace uranus::actor {

    using asio::co_spawn;
    using asio::detached;
    using asio::awaitable;

    RepeatedTimer::RepeatedTimer(ExecutorStrand &strand)
        : strand_(strand),
          innerTimer_(strand_),
          delay_(0),
          rate_(0){
    }

    RepeatedTimer::~RepeatedTimer() {
    }

    void RepeatedTimer::setTask(const RepeatedTimerTask &task) {
        if (running_.test())
            return;

        task_ = task;
    }

    void RepeatedTimer::setActorContext(const shared_ptr<BaseActorContext> &ctx) {
        if (running_.test())
            return;
        context_ = ctx;
    }

    void RepeatedTimer::setDelay(const SteadyDuration delay) {
        if (running_.test())
            return;
        delay_ = delay;
    }

    void RepeatedTimer::setRepeatRate(const SteadyDuration rate) {
        if (running_.test())
            return;
        rate_ = rate;
    }

    void RepeatedTimer::start() {
        if (completed_.test())
            return;

        if (running_.test_and_set(std::memory_order_acq_rel))
            return;

        co_spawn(strand_, [self = shared_from_this()]()-> awaitable<void> {
            if (self->context_.expired())
                co_return;

            const auto ctx = self->context_.lock();
            if (!ctx)
                co_return;

            auto *actor = ctx->getActor();
            if (!actor)
                co_return;

            // TODO
        }, detached);
    }
}
