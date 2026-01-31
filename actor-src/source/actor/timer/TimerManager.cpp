#include "timer/TimerManager.h"
#include "timer/RepeatedTimer.h"
#include "BaseActorContext.h"

#include <ranges>

namespace uranus::actor {
    TimerManager::TimerManager(BaseActorContext &ctx)
        : ctx_(ctx){
    }

    TimerManager::~TimerManager() {

    }

    RepeatedTimerHandle TimerManager::createTimer(
        const RepeatedTask &task,
        const SteadyDuration delay,
        const SteadyDuration rate
    ) {
        if (!ctx_.isRunning())
            return {-1, nullptr};

        const auto id = alloc_.allocate();
        if (timers_.contains(id))
            return { -1, nullptr };

        auto timer = make_shared<RepeatedTimer>(ctx_.shared_from_this(), id);

        timer->setTask(task);
        timer->setDelay(delay);
        timer->setRepeatRate(rate);

        timers_.emplace(id, timer);

        timer->start();
        return { id, timer };
    }

    RepeatedTimerHandle TimerManager::createTimerWithTimepoint(
        const RepeatedTask &task,
        const SteadyTimePoint point,
        const SteadyDuration rate
    ) {
        if (!ctx_.isRunning())
            return {-1, nullptr};

        const auto id = alloc_.allocate();
        if (timers_.contains(id))
            return { -1, nullptr };

        auto timer = make_shared<RepeatedTimer>(ctx_.shared_from_this(), id);

        timer->setTask(task);
        timer->setTimePoint(point);
        timer->setRepeatRate(rate);

        timers_.emplace(id, timer);

        timer->start();
        return { id, timer };
    }

    void TimerManager::cancelTimer(const RepeatedTimerHandle &handle) {
        if (handle.timer) {
            handle.timer->cancel();
        }
    }

    void TimerManager::cancelAll() {
        for (const auto &val: timers_ | std::views::values) {
            val->cancel();
        }
        timers_.clear();
    }

    void TimerManager::removeOnCompleted(const int64_t id) {
        if (!ctx_.isRunning())
            return;

        timers_.erase(id);
    }
}
