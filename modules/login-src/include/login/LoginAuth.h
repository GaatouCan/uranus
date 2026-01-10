#pragma once

#include <actor/ServerModule.h>

namespace uranus::login {

    using actor::ServerModule;

    class LoginAuth final : public ServerModule {

    public:
        SERVER_MODULE_NAME(LoginAuth)
        DISABLE_COPY_MOVE(LoginAuth)

        void start() override;
        void stop() override;
    };
}