#pragma once

#include "actor/actor.export.h"

#include <base/noncopy.h>
#include <base/types.h>
#include <base/IdentAllocator.h>
#include <memory>
#include <unordered_map>
#include <functional>


namespace uranus::actor {

    class BaseActorContext;
    class RepeatedTimer;
    class BaseActor;

    using std::shared_ptr;
    using std::make_shared;
    using std::unordered_map;

    using RepeatedTask = std::function<void(BaseActor *)>;

    struct RepeatedTimerHandle {
        int64_t id;
        shared_ptr<RepeatedTimer> timer;
    };

    class ACTOR_API TimerManager final {

        friend class RepeatedTimer;

    public:
        TimerManager() = delete;

        explicit TimerManager(BaseActorContext &ctx);
        ~TimerManager();

        DISABLE_COPY_MOVE(TimerManager)

        RepeatedTimerHandle createTimer(const RepeatedTask &task, SteadyDuration delay, SteadyDuration rate = SteadyDuration::zero());
        static void cancelTimer(const RepeatedTimerHandle &handle);

        void cancelAll();

    private:
        void removeOnCompleted(int64_t id);

    private:
        BaseActorContext &ctx_;

        IdentAllocator<int64_t, false> alloc_;
        unordered_map<int64_t, shared_ptr<RepeatedTimer>> timers_;
    };
}
