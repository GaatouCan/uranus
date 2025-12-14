#pragma once

#include "actor.export.h"
#include "base/noncopy.h"

namespace uranus::actor {

    class ActorContext;

    class ACTOR_API Actor {

        friend class ActorContext;

    public:
        Actor();
        virtual ~Actor();

        DISABLE_COPY_MOVE(Actor)

        [[nodiscard]] ActorContext *getContext() const;

    private:
        void setContext(ActorContext *ctx);

    private:
        ActorContext *ctx_;
    };
}
