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

    void LoginAuth::onPlayerLogin(Package *pkg) {
        if (pkg->getId() != 1001)
            return;

        Login::ClientLoginRequest req;
        req.ParseFromString(pkg->toString());

        const auto pid = req.player_id();

        const auto *gateway = GET_MODULE(&world_, Gateway);
        if (!gateway) {
            SPDLOG_ERROR("Gateway is nullptr");
            exit(-1);
        }

        if (gateway->hasPlayerLogin(pid)) {
            SPDLOG_WARN("Player login already exists");
            onLoginFailure(pid);
        } else {
            onLoginSuccess(pid);
        }
    }

    void LoginAuth::onLoginSuccess(uint32_t pid) {
    }

    void LoginAuth::onLoginFailure(uint32_t pid) {
    }
} // uranus
