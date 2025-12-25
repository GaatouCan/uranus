#pragma once

#include "Package.h"
#include "base/noncopy.h"

namespace uranus::actor {

    class ActorContext;

    class ACTOR_API BaseActor {

        friend class ActorContext;

    public:
        BaseActor();
        virtual ~BaseActor();

        DISABLE_COPY_MOVE(BaseActor)

        [[nodiscard]] ActorContext *getContext() const;

        virtual void onInitial();
        virtual void onTerminate();

        virtual void onPackage(Envelope &&envelope) = 0;
        virtual PackageHandle onRequest(Envelope &&envelope) = 0;

    private:
        void setContext(ActorContext *ctx);

    private:
        ActorContext *ctx_;
    };
}
