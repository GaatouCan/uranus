#pragma once

#include <actor/ServerModule.h>

namespace uranus {

    using actor::ServerModule;

    class GameWorld;

    class LoginAuth final : public ServerModule {

    public:
        explicit LoginAuth(GameWorld &world);
        ~LoginAuth() override;

        DISABLE_COPY_MOVE(LoginAuth)

        SERVER_MODULE_NAME(LoginAuth)

        [[nodiscard]] GameWorld &getWorld() const;

        void start() override;
        void stop() override;

    private:
        GameWorld &world_;
    };
} // uranus
