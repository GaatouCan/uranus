#include "LoginAuth.h"
#include "LoginProtocol.h"

#include <spdlog/spdlog.h>
#include <network/BaseConnection.h>

#include "login.pb.h"

namespace uranus::login {

    using actor::Package;

    LoginAuth::LoginAuth()
#ifdef LOGIN_DEVELOPER
        : incPlayerId_(1000000)
#endif
    {
        SPDLOG_DEBUG("LoginAuth created");
    }

    LoginAuth::~LoginAuth() {
        SPDLOG_DEBUG("LoginAuth destroyed");
    }

    void LoginAuth::start() {
    }

    void LoginAuth::stop() {
    }

    void LoginAuth::onLoginRequest(PackageHandle &&pkg, const shared_ptr<Connection> &conn) {
        if (pkg->id_ != kLoginRequest)
            return;

        if (conn == nullptr)
            return;

        const auto temp = std::dynamic_pointer_cast<network::BaseConnection>(conn);
        if (temp == nullptr)
            return;

        SPDLOG_INFO("Client[{}] request to login", temp->remoteAddress().to_string());
        ::login::LoginRequest request;

        request.ParseFromArray(pkg->payload_.data(), pkg->payload_.size());

#ifdef LOGIN_DEVELOPER
        int64_t pid = -1;

        // If use developer mode, alloc an increase player_id for the first time login
        if (request.developer_mode() && request.allocate_id()) {
            pid = incPlayerId_.fetch_add(1, std::memory_order_relaxed);
            SPDLOG_INFO("Client[{}] allocated player_id - {}", temp->remoteAddress().to_string(), pid);
        } else {
            pid = request.player_id();
        }
#else
        const auto pid = request.player_id();
#endif

        const auto token = request.token();

        if (pid <= 0) {
            SPDLOG_WARN("Client[{}] authentication failed", temp->remoteAddress().to_string());
            if (onFailure_) {
                std::invoke(onFailure_, conn, pid, "Player ID is invalid");
            }
            return;
        }

        if (token == "LOGIN FAILURE TEST") {
            if (onFailure_) {
                std::invoke(onFailure_, conn, pid, "Token is invalid");
            }
            return;
        }

        if (onSuccess_) {
            SPDLOG_INFO("Client[{}] authentication success", temp->remoteAddress().to_string());
            std::invoke(onSuccess_, conn, pid);
        }
    }

    void LoginAuth::onLogoutRequest(PackageHandle &&pkg, const shared_ptr<Connection> &conn) {
        if (pkg->id_ != kLogoutRequest)
            return;

        if (conn == nullptr)
            return;

        const auto temp = std::dynamic_pointer_cast<network::BaseConnection>(conn);
        if (temp == nullptr)
            return;

        ::login::LogoutRequest req;
        req.ParseFromArray(pkg->payload_.data(), pkg->payload_.size());

        const auto pid = req.player_id();
        const auto reason = req.reason();

        if (onLogout_) {
            SPDLOG_INFO("Client[{} - {}] request to logout", temp->remoteAddress().to_string(), pid);
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

    void LoginAuth::sendLoginSuccess(const shared_ptr<Connection> &conn, const int64_t pid) {
        if (conn == nullptr)
            return;

        auto pkg = Package::getHandle();

        ::login::LoginSuccess res;
        res.set_player_id(pid);

        pkg->setId(kLoginSuccess);
        pkg->setData(res.SerializeAsString());

        conn->sendMessage(std::move(pkg));
    }

    void LoginAuth::sendLoginFailure(const shared_ptr<Connection> &conn, const int64_t pid, const std::string &reason) {
        if (conn == nullptr)
            return;

        auto pkg = Package::getHandle();

        ::login::LoginFailure res;
        res.set_player_id(pid);
        res.set_reason(reason);

        pkg->setId(kLoginFailure);
        pkg->setData(res.SerializeAsString());

        conn->sendMessage(std::move(pkg));
    }

    void LoginAuth::sendLoginPlayerResult(const shared_ptr<Connection> &conn, const int64_t pid, const std::string &message) {
        if (conn == nullptr)
            return;

        auto pkg = Package::getHandle();

        ::login::LoginPlayerResult res;
        res.set_player_id(pid);
        res.set_data(message);

        pkg->setId(kLoginPlayerResult);
        pkg->setData(res.SerializeAsString());

        conn->sendMessage(std::move(pkg));
    }

    void LoginAuth::sendLogoutResponse(const shared_ptr<Connection> &conn, const std::string &reason) {
        if (conn == nullptr)
            return;

        auto pkg = Package::getHandle();

        ::login::LogoutResponse res;
        res.set_data(reason);

        pkg->setId(kLogoutResponse);
        pkg->setData(res.SerializeAsString());

        conn->sendMessage(std::move(pkg));
    }
}
