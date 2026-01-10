#pragma once

#include "login.export.h"
#include <actor/ServerModule.h>

namespace uranus::login {

    using actor::ServerModule;

    class LOGIN_API LoginAuth final : public ServerModule {

    public:
        SERVER_MODULE_NAME(LoginAuth)
        DISABLE_COPY_MOVE(LoginAuth)

        void start() override;
        void stop() override;
    };
}