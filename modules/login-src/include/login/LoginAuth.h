#pragma once

#include "login.export.h"

#include <actor/ServerModule.h>
#include <actor/Package.h>

namespace uranus::login {

    using actor::ServerModule;
    using actor::PackageHandle;

    class LOGIN_API LoginAuth final : public ServerModule {

    public:
        SERVER_MODULE_NAME(LoginAuth)
        DISABLE_COPY_MOVE(LoginAuth)

        void start() override;
        void stop() override;

        void onLoginRequest(PackageHandle &&pkg);
    };
}