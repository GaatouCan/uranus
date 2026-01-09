#pragma once

#include "Package.h"
#include "base/noncopy.h"

namespace uranus::actor {

    class BaseActorContext;

    class ACTOR_API BaseActor {

        friend class BaseActorContext;

    public:
        BaseActor();
        virtual ~BaseActor();

        DISABLE_COPY_MOVE(BaseActor)

        [[nodiscard]] BaseActorContext *getContext() const;

        virtual void onInitial();
        virtual void onTerminate();

        virtual void onPackage(Envelope &&envelope) = 0;
        virtual PackageHandle onRequest(Envelope &&envelope) = 0;

    private:
        void setContext(BaseActorContext *ctx);

    private:
        BaseActorContext *ctx_;
    };
}


# if defined(_WIN32) || defined(_WIN64)
#   define ACTOR_EXPORT extern "C" __declspec(dllexport)
# else
#   define ACTOR_EXPORT extern "C" __attribute__((visibility("default")))
# endif


