#pragma once

#include "actor/actor.export.h"

#include <base/types.h>
#include <base/noncopy.h>
#include <memory>
#include <atomic>
#include <functional>


namespace uranus::actor {

    class BaseActor;
    class BaseActorContext;

    using std::shared_ptr;
    using std::weak_ptr;
    using std::enable_shared_from_this;
    using std::atomic_flag;

    using RepeatedTask = std::function<void(BaseActor *)>;


    class ACTOR_API RepeatedTimer final : public enable_shared_from_this<RepeatedTimer> {

        friend class TimerManager;

    public:
        RepeatedTimer() = delete;

        RepeatedTimer(const shared_ptr<BaseActorContext> &owner, int64_t id);
        ~RepeatedTimer();

        DISABLE_COPY_MOVE(RepeatedTimer)

        // 所有这些操作只允许TimerManager来调用 外部只能观察到有一个Timer
    private:
        void setTask(const RepeatedTask &task);

        void setDelay(SteadyDuration delay);
        void setTimePoint(SteadyTimePoint point);

        void setRepeatRate(SteadyDuration rate);

        void start();
        void cancel();

    private:
        asio::any_io_executor exec_;
        const int64_t id_;

        weak_ptr<BaseActorContext> owner_;

        SteadyTimer innerTimer_;
        RepeatedTask task_;

        SteadyDuration delay_;
        SteadyTimePoint point_;
        SteadyDuration rate_;

        atomic_flag running_;
        atomic_flag completed_;
    };
}
