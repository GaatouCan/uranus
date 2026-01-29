#pragma once

#include "actor.export.h"

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

    using RepeatedTimerTask = std::function<void(BaseActor *)>;

    class ACTOR_API RepeatedTimer final : public enable_shared_from_this<RepeatedTimer> {

    public:
        RepeatedTimer() = delete;

        explicit RepeatedTimer(ExecutorStrand &strand);
        ~RepeatedTimer();

        DISABLE_COPY_MOVE(RepeatedTimer)

        void setTask(const RepeatedTimerTask &task);

        void setActorContext(const shared_ptr<BaseActorContext> &ctx);
        void setDelay(SteadyDuration delay);
        void setRepeatRate(SteadyDuration rate);

        void start();

    private:
        ExecutorStrand &strand_;

        SteadyTimer innerTimer_;
        RepeatedTimerTask task_;

        SteadyDuration delay_;
        SteadyDuration rate_;

        weak_ptr<BaseActorContext> context_;
        atomic_flag running_;
        atomic_flag completed_;
    };
}