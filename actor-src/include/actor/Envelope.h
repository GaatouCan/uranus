#pragma once

#include "Package.h"
#include "DataAsset.h"

#include <variant>

namespace uranus::actor {

    using DataAssetHandle = std::unique_ptr<DataAsset>;

    struct ACTOR_API Envelope final {

        enum EnvelopeType {
            kFromClient     = 1,
            kFromPlayer     = 1 << 1,
            kFromService    = 1 << 2,
            kToClient       = 1 << 3,
            kToPlayer       = 1 << 4,
            kToService      = 1 << 5,
            kRequest        = 1 << 6,
            kResponse       = 1 << 7,
            kEvent          = 1 << 8,
        };

        using VariantHandle = std::variant<PackageHandle, DataAssetHandle>;

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

        Envelope(const Envelope &) = delete;
        Envelope &operator=(const Envelope &) = delete;

        Envelope(Envelope &&rhs) noexcept;
        Envelope &operator=(Envelope &&rhs) noexcept;
    };
}