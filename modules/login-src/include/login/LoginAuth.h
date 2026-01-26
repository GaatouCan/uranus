#pragma once

#include "login.export.h"

#include <actor/ServerModule.h>
#include <actor/Package.h>
#include <functional>

#ifdef LOGIN_DEVELOPER
#include <atomic>
#endif


namespace uranus::network {
    class Connection;
}

namespace uranus::login {

    using actor::ServerModule;
    using actor::PackageHandle;
    using network::Connection;
    using std::function;
    using std::shared_ptr;

    class LOGIN_API LoginAuth final : public ServerModule {

    public:
        using SuccessCallback   = std::function<void(const shared_ptr<Connection> &, int64_t)>;
        using FailureCallback   = std::function<void(const shared_ptr<Connection> &, int64_t, const std::string &)>;
        using LogoutCallback    = std::function<void(const shared_ptr<Connection> &, int64_t, const std::string &)>;

        LoginAuth();
        ~LoginAuth() override;

        SERVER_MODULE_NAME(LoginAuth)
        DISABLE_COPY_MOVE(LoginAuth)

        void start() override;
        void stop() override;

        void onLoginRequest(PackageHandle &&pkg, const shared_ptr<Connection> &conn);
        void onLogoutRequest(PackageHandle &&pkg, const shared_ptr<Connection> &conn);

        void onLoginSuccess(const SuccessCallback &cb);
        void onLoginFailure(const FailureCallback &cb);
        void onPlayerLogout(const LogoutCallback &cb);

        static void sendLoginSuccess(const shared_ptr<Connection> &conn, int64_t pid);
        static void sendLoginFailure(const shared_ptr<Connection> &conn, int64_t pid, const std::string &reason);
        static void sendLoginPlayerResult(const shared_ptr<Connection> &conn, int64_t pid, const std::string &message);

        static void sendLogoutResponse(const shared_ptr<Connection> &conn, const std::string &reason);

    private:
        SuccessCallback onSuccess_;
        FailureCallback onFailure_;
        LogoutCallback onLogout_;

#ifdef LOGIN_DEVELOPER
        std::atomic_int64_t incPlayerId_;
#endif
    };
}