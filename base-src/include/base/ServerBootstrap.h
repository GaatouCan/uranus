#pragma once

#include "base.export.h"
#include "noncopy.h"

namespace uranus {

    class BASE_API ServerBootstrap {

    public:
        ServerBootstrap();
        virtual ~ServerBootstrap();

        DISABLE_COPY_MOVE(ServerBootstrap)

        virtual void run() = 0;
        virtual void terminate() = 0;
    };
}