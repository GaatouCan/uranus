#pragma once

namespace uranus::login {
    enum LoginProtocolID {
        kLoginRequest = 1001,
        kLoginSuccess = 1002,
        kLoginFailure = 1003,
        kLogoutRequest = 1004,
        kLogoutResponse = 1005,
        kHeartbeat = 1006,
        kLoginDataResult = 1051,
    };
}