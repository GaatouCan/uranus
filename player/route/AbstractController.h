#pragma once

#include "Common.h"

namespace protocol {
    class AbstractController {

    public:
        AbstractController() = default;
        virtual ~AbstractController() = default;

        DISABLE_COPY_MOVE(AbstractController)
    };
}
