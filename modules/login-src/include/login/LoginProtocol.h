#pragma once

namespace uranus::login {
    enum LoginProtocolID {
        kLoginRequest = 1001,
        kLoginSuccess = 1002,
        kLoginFailure = 1003,
        kLoginPlayerResult = 1004,
        kLogoutRequest = 1005,
        kLogoutResponse = 1006,
        kHeartbeat = 1007,
    };
}