#pragma once

#include "Common.h"

#include <cstdint>

namespace uranus {

    enum MessageType {
        kToService  = 1,
        kToPlayer   = 1 << 1,
        kToClient   = 1 << 2,
        kToServer   = 1 << 3,
    };

    struct CORE_API Message final {
        int32_t type;

        int32_t session;
        int64_t source;

        void *  data;
        size_t  length;
    };
}
