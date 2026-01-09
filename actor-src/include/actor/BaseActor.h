#pragma once

#include "Package.h"
#include "base/noncopy.h"

namespace uranus::actor {

    class ActorContext;

    class ACTOR_API BaseActor {

    public:
        BaseActor();
        virtual ~BaseActor();

        DISABLE_COPY_MOVE(BaseActor)

        [[nodiscard]] ActorContext *getContext() const;

        virtual void onInitial(ActorContext *ctx);
        virtual void onTerminate();

        virtual void onPackage(PackageHandle &&pkg) = 0;
        virtual PackageHandle onRequest(PackageHandle &&pkg) = 0;

    private:
        ActorContext *ctx_;
    };
}


# if defined(_WIN32) || defined(_WIN64)
#   define ACTOR_EXPORT extern "C" __declspec(dllexport)
# else
#   define ACTOR_EXPORT extern "C" __attribute__((visibility("default")))
# endif


