#pragma once

#include "Common.h"

#include <cstdint>

namespace uranus {

    struct CORE_API Message final {

        enum MessageType {
            kToService      = 1,
            kToPlayer       = 1 << 1,
            kToClient       = 1 << 2,
            kToServer       = 1 << 3,
            kFromClient     = 1 << 4,
            kFromPlayer     = 1 << 5,
            kFromService    = 1 << 6,
            kFromServer     = 1 << 7,
            kRequest        = 1 << 8,
            kResponse       = 1 << 9,
        };

        int32_t type;

        int32_t session;
        int64_t source;

        void *  data;
        size_t  length;
    };
}
