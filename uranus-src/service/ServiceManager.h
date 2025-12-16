#pragma once

#include <actor/ServerModule.h>

namespace uranus {

    using actor::ServerModule;
    class GameWorld;

    class ServiceManager final : public ServerModule {

    public:
        explicit ServiceManager(GameWorld &world);
        ~ServiceManager() override;

        SERVER_MODULE_NAME(ServiceManager)

        void start() override;
        void stop() override;

    private:
        GameWorld &world_;
    };
} // uranus
