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

    using ActorTask = std::function<void(BaseActor *)>;

    struct ACTOR_API Envelope final {

        enum EnvelopeTag {
            kPackage    = 1,
            kDataAsset  = 2,
            kTickInfo   = 3,
            kTask       = 4
        };

        enum EnvelopeType {
            kFromClient     = 1,
            kFromPlayer     = 1 << 1,
            kFromService    = 1 << 2,
            kToClient       = 1 << 3,
            kToPlayer       = 1 << 4,
            kToService      = 1 << 5,
            kRequest        = 1 << 6,
            kResponse       = 1 << 7,
        };

        using VariantHandle = std::variant<
            PackageHandle,
            DataAssetHandle,
            ActorTickInfo,
            ActorTask
        >;

        int32_t tag;

        int32_t type;
        int64_t source;

        union {
            int64_t session;
            int64_t event;
        };

        VariantHandle variant;

        Envelope();

        Envelope(int32_t ty, int64_t src, PackageHandle &&pkg);
        Envelope(int32_t ty, int64_t src, int64_t sess, PackageHandle &&pkg);

        Envelope(int32_t ty, int64_t src, int64_t evt, DataAssetHandle &&data);

        Envelope(SteadyTimePoint now, SteadyDuration delta);

        Envelope(const Envelope &) = delete;
        Envelope &operator=(const Envelope &) = delete;

        Envelope(Envelope &&rhs) noexcept;
        Envelope &operator=(Envelope &&rhs) noexcept;
    };
}