#pragma once

#include <actor/ServerModule.h>

namespace uranus {

    using actor::ServerModule;

    class GameWorld;

    class WorldMonitor final : public ServerModule {

    public:
        WorldMonitor()= delete;

        explicit WorldMonitor(GameWorld &world);
        ~WorldMonitor() override;

        SERVER_MODULE_NAME(Monitor)
        DISABLE_COPY_MOVE(WorldMonitor)

        void start() override;
        void stop() override;

    private:
        GameWorld &world_;
    };
} // uranus
