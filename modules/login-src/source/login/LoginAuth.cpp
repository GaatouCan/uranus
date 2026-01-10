#include "LoginAuth.h"

#include "login.pb.h"

namespace uranus::login {

    void LoginAuth::start() {
    }

    void LoginAuth::stop() {
    }

    void LoginAuth::onLoginRequest(PackageHandle &&pkg) {
        if (pkg->id_ != 1001)
            return;

        ::login::LoginRequest request;

        request.ParseFromArray(pkg->payload_.data(), pkg->payload_.size());

        const auto pid = request.player_id();

        if (pid <= 0) {
            // TODO: failed
            return;
        }

        // TODO: Success
    }
}
