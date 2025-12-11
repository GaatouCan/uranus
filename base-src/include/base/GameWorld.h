#pragma once

#include "base.export.h"
#include "noncopy.h"

namespace uranus {
    class BASE_API GameWorld {

    public:
        GameWorld() = default;
        virtual ~GameWorld() = default;

        DISABLE_COPY_MOVE(GameWorld)

        virtual void run() = 0;
        virtual void terminate() = 0;
    };
}