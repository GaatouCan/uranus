#include "LoginAuth.h"

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
    }
} // uranus
