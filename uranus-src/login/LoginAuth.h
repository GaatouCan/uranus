#pragma once

#include <actor/Package.h>
#include <actor/ServerModule.h>

namespace uranus {

    using actor::ServerModule;
    using actor::Package;

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

        void onPlayerLogin(Package *pkg);

        void onLoginSuccess(uint32_t pid);
        void onLoginFailure(uint32_t pid);

    private:
        GameWorld &world_;
    };
} // uranus
