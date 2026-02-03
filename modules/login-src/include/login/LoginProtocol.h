#pragma once

namespace uranus::login {
    enum LoginProtocolID {
        kLoginRequest = 1001,
        kLoginSuccess = 1002,
        kLoginFailure = 1003,
        kLoginRepeated = 1004,
        kLoginProcessInfo = 1005,
        kLogoutRequest = 1006,
        kLogoutResponse = 1007,
        kHeartbeat = 1008,
    };
}