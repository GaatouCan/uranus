#pragma once

#include "actor.export.h"
#include <base/noncopy.h>


namespace uranus::actor {

    class ACTOR_API ServerModule {

    public:
        ServerModule();
        virtual ~ServerModule();

        DISABLE_COPY_MOVE(ServerModule)

        virtual const char *getModuleName() = 0;

        virtual void start() = 0;
        virtual void stop() = 0;
    };
}