#include "RepeatedTimer.h"
#include "BaseActorContext.h"

#include <asio.hpp>

namespace uranus::actor {

    using asio::co_spawn;
    using asio::detached;
    using asio::awaitable;

    RepeatedTimer::RepeatedTimer(const shared_ptr<BaseActorContext> &ctx, const int64_t id)
        : context_(ctx),
          id_(id),
          innerTimer_(ctx->strand_),
          delay_(0),
          rate_(0) {
    }

    RepeatedTimer::~RepeatedTimer() {
    }

    void RepeatedTimer::setTask(const RepeatedTask &task) {
        if (running_.test(std::memory_order_acquire))
            return;

        task_ = task;
    }

    void RepeatedTimer::setDelay(const SteadyDuration delay) {
        if (running_.test(std::memory_order_acquire))
            return;

        delay_ = delay;
    }

    void RepeatedTimer::setRepeatRate(const SteadyDuration rate) {
        if (running_.test(std::memory_order_acquire))
            return;

        rate_ = rate;
    }

    void RepeatedTimer::start() {
        if (completed_.test(std::memory_order_acquire))
            return;

        if (running_.test_and_set(std::memory_order_acq_rel))
            return;

        const auto ctx = context_.lock();
        if (!ctx)
            return;

        co_spawn(ctx->strand_, [self = shared_from_this()]()-> awaitable<void> {
            if (self->context_.expired()) {
                self->completed_.test_and_set(std::memory_order_release);
                co_return;
            }

            if (self->completed_.test(std::memory_order_acquire))
                co_return;

            auto point = std::chrono::steady_clock::now();
            point += self->delay_;

            auto kCleanUp = [&] {
                self->completed_.test_and_set(std::memory_order_release);
                if (const auto temp = self->context_.lock(); temp && self->id_ > 0) {
                    temp->timerManager_.removeOnCompleted(self->id_);
                    self->id_ = -1;
                }
            };

            // The first await
            {
                self->innerTimer_.expires_at(point);
                if (const auto [ec] = co_await self->innerTimer_.async_wait(); ec) {
                    if (ec != asio::error::operation_aborted) {
                        // TODO: Not cancel error_Code
                    }

                    kCleanUp();
                    co_return;
                }

                if (const auto temp = self->context_.lock()) {
                    auto evl = Envelope::makeCallback(self->task_);
                    temp->pushEnvelope(std::move(evl));
                } else {
                    kCleanUp();
                    co_return;
                }
            }

            // The loop
            if (self->rate_ > SteadyDuration::zero()) {
                while (true) {
                    point += self->rate_;
                    self->innerTimer_.expires_at(point);

                    if (const auto [ec] = co_await self->innerTimer_.async_wait(); ec) {
                        if (ec != asio::error::operation_aborted) {
                            // TODO: Not cancel error_Code
                        }
                        break;
                    }

                    if (const auto temp = self->context_.lock()) {
                        auto evl = Envelope::makeCallback(self->task_);
                        temp->pushEnvelope(std::move(evl));
                    } else {
                        kCleanUp();
                        co_return;
                    }
                }
            }

            kCleanUp();
        }, detached);
    }

    void RepeatedTimer::cancel() {
        if (completed_.test_and_set(std::memory_order_acq_rel))
            return;

        if (!running_.test(std::memory_order_acquire) && id_ > 0) {
            if (const auto ctx = context_.lock()) {
                asio::dispatch(ctx->strand_, [self = shared_from_this()]() mutable {
                    if (const auto temp = self->context_.lock(); temp && self->id_ > 0) {
                        temp->timerManager_.removeOnCompleted(self->id_);
                        self->id_ = -1;
                    }
                });
            }

            return;
        }

        innerTimer_.cancel();
    }
}

