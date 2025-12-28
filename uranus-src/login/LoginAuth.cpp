#include "LoginAuth.h"

#include <login.pb.h>

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
        // req.ParseFromString(pkg->toString());

        const auto pid = req.player_id();
    }
} // uranus
