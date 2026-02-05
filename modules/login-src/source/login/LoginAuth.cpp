#include "LoginAuth.h"
#include "LoginProtocol.h"

#include <spdlog/spdlog.h>
#include <network/BaseConnection.h>

#include "login.pb.h"

namespace uranus::login {

    using actor::Package;

    LoginAuth::LoginAuth(asio::any_io_executor exec)
        : exec_(std::move(exec))
#ifdef LOGIN_DEVELOPER
        , incPlayerId_(1000000)
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
                asio::post(exec_, [func = onFailure_, conn, pid] {
                    std::invoke(func, conn, pid, "Player ID is invalid");
                });
            }
            return;
        }

        if (token == "LOGIN FAILURE TEST") {
            if (onFailure_) {
                asio::post(exec_, [func = onFailure_, conn, pid] {
                    std::invoke(func, conn, pid, "Token is invalid");
                });
            }
            return;
        }

        if (onSuccess_) {
            SPDLOG_INFO("Client[{}] authentication success", temp->remoteAddress().to_string());
            asio::post(exec_, [func = onSuccess_, conn, pid] {
                std::invoke(func, conn, pid);
            });
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
            asio::post(exec_, [func = onLogout_, conn, pid, reason] {
                std::invoke(func, conn, pid, reason);
            });
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

    void LoginAuth::sendLoginSuccess(
        const shared_ptr<Connection> &conn,
        const int64_t pid
    ) {
        if (conn == nullptr)
            return;

        auto pkg = Package::getHandle();

        ::login::LoginSuccess res;
        res.set_player_id(pid);

        pkg->setId(kLoginSuccess);

        pkg->payload_.resize(res.ByteSizeLong());
        res.SerializeToArray(pkg->payload_.data(), pkg->payload_.size());

        conn->sendMessage(std::move(pkg));
    }

    void LoginAuth::sendLoginFailure(
        const shared_ptr<Connection> &conn,
        const int64_t pid,
        const std::string &reason
    ) {
        if (conn == nullptr)
            return;

        auto pkg = Package::getHandle();

        ::login::LoginFailure res;
        res.set_player_id(pid);
        res.set_reason(reason);

        pkg->setId(kLoginFailure);

        pkg->payload_.resize(res.ByteSizeLong());
        res.SerializeToArray(pkg->payload_.data(), pkg->payload_.size());

        conn->sendMessage(std::move(pkg));
    }

    void LoginAuth::sendLoginRepeated(
        const shared_ptr<Connection> &conn,
        const int64_t pid,
        const std::string &address
    ) {
        if (conn == nullptr)
            return;

        auto pkg = Package::getHandle();

        ::login::LoginRepeated res;
        res.set_player_id(pid);
        res.set_address(address);

        pkg->setId(kLoginRepeated);

        pkg->payload_.resize(res.ByteSizeLong());
        res.SerializeToArray(pkg->payload_.data(), pkg->payload_.size());

        conn->sendMessage(std::move(pkg));
    }

    void LoginAuth::sendLoginProcessInfo(
        const shared_ptr<Connection> &conn,
        const int64_t pid,
        const std::string &message
    ) {
        if (conn == nullptr)
            return;

        auto pkg = Package::getHandle();

        ::login::LoginProcessInfo info;
        info.set_player_id(pid);
        info.set_data(message);

        pkg->setId(kLoginProcessInfo);

        pkg->payload_.resize(info.ByteSizeLong());
        info.SerializeToArray(pkg->payload_.data(), pkg->payload_.size());

        conn->sendMessage(std::move(pkg));
    }

    void LoginAuth::sendLogoutResponse(const shared_ptr<Connection> &conn, const std::string &reason) {
        if (conn == nullptr)
            return;

        auto pkg = Package::getHandle();

        ::login::LogoutResponse res;
        res.set_data(reason);

        pkg->setId(kLogoutResponse);


        pkg->payload_.resize(res.ByteSizeLong());
        res.SerializeToArray(pkg->payload_.data(), pkg->payload_.size());

        conn->sendMessage(std::move(pkg));
    }
}
