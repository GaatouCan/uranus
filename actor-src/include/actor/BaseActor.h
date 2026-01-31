#pragma once

#include "Package.h"
#include "base/noncopy.h"

#include <chrono>

namespace uranus::actor {

    class ActorContext;
    class BaseActorContext;
    class DataAsset;

    using SteadyTimePoint = std::chrono::steady_clock::time_point;
    using SteadyDuration = std::chrono::steady_clock::duration;
    using DataAssetHandle = std::unique_ptr<DataAsset>;

    class ACTOR_API BaseActor {

        friend class BaseActorContext;

    public:
        BaseActor();
        virtual ~BaseActor();

        DISABLE_COPY_MOVE(BaseActor)

        [[nodiscard]] ActorContext *getContext() const;

        virtual void onInitial(ActorContext *ctx);

        virtual void onStart(DataAsset *data);
        virtual void onTerminate();

        virtual void onPackage(int64_t src, PackageHandle &&pkg) = 0;
        virtual void onEvent(int64_t evt, DataAsset *data) = 0;

        virtual PackageHandle onRequest(int64_t src, PackageHandle &&req) = 0;

        virtual void onTick(SteadyTimePoint now, SteadyDuration delta);

    private:
        ActorContext *ctx_;

    protected:
        bool enableTick_;
    };

    inline constexpr auto kUranusActorABIVersion = 1;
    inline constexpr auto kUranusActorAPIVersion = 1;
    inline constexpr auto kUranusActorHeaderVersion = 1;

    struct ActorVersion final {
        uint32_t abi_version;
        uint32_t api_version;
        uint32_t header_version;
    };
}

#define ACTOR_GET_MODULE(module) \
    getContext()->getModuleT<module>(#module)


# if defined(_WIN32) || defined(_WIN64)
#   define ACTOR_EXPORT extern "C" __declspec(dllexport)
# else
#   define ACTOR_EXPORT extern "C" __attribute__((visibility("default")))
# endif

#define EXPORT_ACTOR_VERSION                                        \
ACTOR_EXPORT const uranus::actor::ActorVersion *GetActorVersion() { \
    using namespace uranus::actor;                                  \
    static constexpr ActorVersion ver {                             \
        kUranusActorABIVersion,                                     \
        kUranusActorAPIVersion,                                     \
        kUranusActorHeaderVersion                                   \
    };                                                              \
    return &ver;                                                    \
}