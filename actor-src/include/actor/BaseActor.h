#pragma once

#include "Package.h"
#include "base/noncopy.h"

namespace uranus::actor {

    class ActorContext;
    class DataAsset;

    class ACTOR_API BaseActor {

    public:
        BaseActor();
        virtual ~BaseActor();

        DISABLE_COPY_MOVE(BaseActor)

        [[nodiscard]] ActorContext *getContext() const;

        virtual void onInitial(ActorContext *ctx);

        virtual void onStart(DataAsset *data);
        virtual void onTerminate();

        virtual void onPackage(PackageHandle &&pkg) = 0;
        virtual void onEvent(int64_t evt, DataAsset *data) = 0;

        virtual PackageHandle onRequest(PackageHandle &&req) = 0;

    private:
        ActorContext *ctx_;
    };
}

#define ACTOR_GET_MODULE(module) \
    getContext()->getModuleT<module>(#module)


# if defined(_WIN32) || defined(_WIN64)
#   define ACTOR_EXPORT extern "C" __declspec(dllexport)
# else
#   define ACTOR_EXPORT extern "C" __attribute__((visibility("default")))
# endif


