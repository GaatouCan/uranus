#include "timer/RepeatedTimer.h"
#include "BaseActorContext.h"

#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>

namespace uranus::actor {

    using asio::co_spawn;
    using asio::detached;
    using asio::awaitable;

    RepeatedTimer::RepeatedTimer(const shared_ptr<BaseActorContext> &owner, const int64_t id)
        : exec_(owner->executor()),
          id_(id),
          owner_(owner),
          innerTimer_(exec_),
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

    void RepeatedTimer::setTimePoint(const SteadyTimePoint point) {
        if (running_.test(std::memory_order_acquire))
            return;
        point_ = point;
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

        co_spawn(exec_, [self = shared_from_this()]()-> awaitable<void> {
            try {
                if (self->owner_.expired()) {
                    self->completed_.test_and_set(std::memory_order_release);
                    co_return;
                }

                if (self->completed_.test(std::memory_order_acquire))
                    co_return;

                auto point = std::chrono::steady_clock::now();

                if (self->delay_ > SteadyDuration::zero()) {
                    point += self->delay_;
                } else if (self->point_ >= point) {
                    point = self->point_;
                }

                auto kCleanUp = [self] {
                    self->completed_.test_and_set(std::memory_order_release);
                    if (const auto temp = self->owner_.lock()) {
                        temp->timerManager_.removeOnCompleted(self->id_);
                    }
                };

                // The first await
                {
                    self->innerTimer_.expires_at(point);
                    const auto [ec] = co_await self->innerTimer_.async_wait();

                    const auto temp = self->owner_.lock();
                    if (temp == nullptr) {
                        self->completed_.test_and_set(std::memory_order_release);
                        co_return;
                    }

                    if (ec) {
                        if (ec != asio::error::operation_aborted)
                            temp->onErrorCode(ec);

                        kCleanUp();
                        co_return;
                    }

                    auto evl = Envelope::makeCallback(self->task_);
                    temp->pushEnvelope(std::move(evl));
                }

                // The loop
                if (self->rate_ > SteadyDuration::zero()) {
                    while (true) {
                        point += self->rate_;
                        self->innerTimer_.expires_at(point);

                        const auto [ec] = co_await self->innerTimer_.async_wait();

                        const auto temp = self->owner_.lock();
                        if (temp == nullptr) {
                            self->completed_.test_and_set(std::memory_order_release);
                            co_return;
                        }

                        if (ec) {
                            if (ec != asio::error::operation_aborted)
                                temp->onErrorCode(ec);

                            break;
                        }

                        auto evl = Envelope::makeCallback(self->task_);
                        temp->pushEnvelope(std::move(evl));
                    }
                }

                kCleanUp();
            } catch (std::exception &e) {
                if (const auto temp = self->owner_.lock()) {
                    temp->onException(e);
                }
            }
        }, detached);
    }

    void RepeatedTimer::cancel() {
        if (completed_.test_and_set(std::memory_order_acq_rel))
            return;

        if (!running_.test_and_set(std::memory_order_acq_rel)) {
            asio::dispatch(exec_, [self = shared_from_this()]() mutable {
                if (const auto temp = self->owner_.lock(); temp && self->id_ > 0) {
                    temp->timerManager_.removeOnCompleted(self->id_);
                }
            });
            return;
        }

        innerTimer_.cancel();
    }
}
