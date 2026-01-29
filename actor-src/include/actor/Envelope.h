#pragma once

#include "Package.h"
#include "DataAsset.h"

#include <variant>
#include <chrono>
#include <functional>

namespace uranus::actor {

    class BaseActor;

    using DataAssetHandle = std::unique_ptr<DataAsset>;
    using SteadyTimePoint = std::chrono::steady_clock::time_point;
    using SteadyDuration = std::chrono::steady_clock::duration;

    struct ActorTickInfo {
        SteadyTimePoint now;
        SteadyDuration delta;
    };

    using ActorCallback = std::function<void(BaseActor *)>;

    struct ACTOR_API Envelope final {

        enum EnvelopeType {
            kPackage    = 1,
            kRequest    = 2,
            kResponse   = 3,
            kDataAsset  = 4,
            kTickInfo   = 5,
            kCallback   = 6
        };

        using VariantHandle = std::variant<
            PackageHandle,
            DataAssetHandle,
            ActorTickInfo,
            ActorCallback>;

        int32_t type;
        int32_t flag;

        int64_t source;

        union {
            int64_t session;
            int64_t event;
        };

        VariantHandle variant;

        Envelope();

        Envelope(const Envelope &) = delete;
        Envelope &operator=(const Envelope &) = delete;

        Envelope(Envelope &&rhs) noexcept;
        Envelope &operator=(Envelope &&rhs) noexcept;

        static Envelope makePackage(int flag, int64_t src, PackageHandle &&pkg);

        static Envelope makeRequest(int flag, int64_t src, int64_t sess, PackageHandle &&req);
        static Envelope makeResponse(int flag, int64_t src, int64_t sess, PackageHandle &&res);

        static Envelope makeDataAsset(int64_t evt, DataAssetHandle &&data);
        static Envelope makeTickInfo(SteadyTimePoint now, SteadyDuration delta);
        static Envelope makeCallback(ActorCallback &&cb);
    };
}