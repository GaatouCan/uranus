#include "LoginAuth.h"

#include "GameWorld.h"
#include "gateway/Gateway.h"

#include <login.pb.h>
#include <spdlog/spdlog.h>

namespace uranus {
    LoginAuth::LoginAuth(GameWorld &world)
        : world_(world) {
    }

    LoginAuth::~LoginAuth() {
    }

    GameWorld &LoginAuth::getWorld() const {
        return world_;
    }

    void LoginAuth::start() {
    }

    void LoginAuth::stop() {
    }

    void LoginAuth::onPlayerLogin(Package *pkg, const std::string &key) {
        if (pkg->getId() != 1001)
            return;

        Login::ClientLoginRequest req;
        req.ParseFromString(pkg->toString());

        const auto pid = req.player_id();
        onLoginSuccess(pid, key);
    }

    void LoginAuth::onLoginSuccess(uint32_t pid, const std::string &key) {
        auto *gateway = GET_MODULE(&world_, Gateway);
        if (!gateway) {
            SPDLOG_ERROR("Gateway is nullptr");
            exit(-1);
        }

        gateway->onPlayerLogin(pid, key);
    }

    void LoginAuth::onLoginFailure(uint32_t pid, const std::string &key) {
        auto *gateway = GET_MODULE(&world_, Gateway);
        if (!gateway) {
            SPDLOG_ERROR("Gateway is nullptr");
            exit(-1);
        }

        if (const auto conn = gateway->find(key)) {
            SPDLOG_WARN("Connection[{}] login request failed", key);
            // TODO
        }
    }
} // uranus
