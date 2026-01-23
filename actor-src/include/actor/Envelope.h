#pragma once

#include "Package.h"

namespace uranus::actor {
    struct ACTOR_API Envelope final {
        int32_t type;

        int64_t source;
        int64_t session;

        PackageHandle package;

        Envelope();

        Envelope(int32_t ty, int64_t src, PackageHandle &&pkg);
        Envelope(int32_t ty, int64_t src, int64_t sess, PackageHandle &&pkg);

        Envelope(const Envelope &) = delete;
        Envelope &operator=(const Envelope &) = delete;

        Envelope(Envelope &&rhs) noexcept;
        Envelope &operator=(Envelope &&rhs) noexcept;
    };
}