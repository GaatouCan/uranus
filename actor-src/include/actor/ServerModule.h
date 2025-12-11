#pragma once

#include "actor.export.h"
#include <base/noncopy.h>

namespace uranus {
    class GameWorld;
}

namespace uranus::actor {

    class ACTOR_API ServerModule {

    public:
        ServerModule() = delete;

        explicit ServerModule(GameWorld &world);
        virtual ~ServerModule();

        DISABLE_COPY_MOVE(ServerModule)

        virtual const char *getModuleName() = 0;

        [[nodiscard]] GameWorld &getWorld() const;

        virtual void start() = 0;
        virtual void stop() = 0;

    private:
        GameWorld &world_;
    };
}