#include "LoginAuth.h"
#include "LoginProtocol.h"

#include <network/Connection.h>

#include "login.pb.h"

namespace uranus::login {

    using actor::Package;

    LoginAuth::LoginAuth() {
    }

    LoginAuth::~LoginAuth() {
    }

    void LoginAuth::start() {
    }

    void LoginAuth::stop() {
    }

    void LoginAuth::onLoginRequest(PackageHandle &&pkg, const shared_ptr<Connection> &conn) {
        if (pkg->id_ != kLoginRequest)
            return;

        ::login::LoginRequest request;

        request.ParseFromArray(pkg->payload_.data(), pkg->payload_.size());

        const auto pid = request.player_id();
        const auto token = request.token();

        if (pid <= 0) {
            if (onFailure_) {
                std::invoke(onFailure_, conn, pid, "Token is invalid");
            }
            return;
        }

        if (onSuccess_) {
            std::invoke(onSuccess_, conn, pid);
        }
    }

    void LoginAuth::onLogoutRequest(PackageHandle &&pkg, const shared_ptr<Connection> &conn) {
        if (pkg->id_ != kLogoutRequest)
            return;

        ::login::LogoutRequest req;
        req.ParseFromArray(pkg->payload_.data(), pkg->payload_.size());

        const auto pid = req.player_id();
        const auto reason = req.reason();

        if (onLogout_) {
            std::invoke(onLogout_, conn, pid, reason);
        }
    }

    void LoginAuth::onLoginSuccess(const SuccessCallback &cb) {
        onSuccess_ = cb;
    }

    void LoginAuth::onLoginFailure(const FailureCallback &cb) {
        onFailure_ = cb;
    }

    void LoginAuth::onPlayerLogout(const LogoutCallback &cb) {
        onLogout_ = cb;
    }

    PackageHandle LoginAuth::PackLoginSuccess(const int64_t pid) {
        auto pkg = Package::getHandle();

        ::login::LoginSuccess res;
        res.set_player_id(pid);

        pkg->setId(kLoginSuccess);
        pkg->setData(res.SerializeAsString());

        return pkg;
    }

    PackageHandle LoginAuth::PackLoginFailure(const int64_t pid, const std::string &reason) {
        auto pkg = Package::getHandle();

        ::login::LoginFailure res;
        res.set_player_id(pid);
        res.set_reason(reason);

        pkg->setId(kLoginFailure);
        pkg->setData(res.SerializeAsString());

        return pkg;
    }

    PackageHandle LoginAuth::PackLogoutResponse(const std::string &reason) {
        auto pkg = Package::getHandle();

        ::login::LogoutResponse res;
        res.set_data(reason);

        pkg->setId(kLogoutResponse);
        pkg->setData(res.SerializeAsString());

        return pkg;
    }
}
