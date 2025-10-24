#pragma once

enum class FixedPackageID : int {
    kLoginRequest       = 1001,
    kLoginSuccess       = 1002,
    kLoginFailed        = 1003,
    kLoginRepeated      = 1004,
    kHeartbeat          = 1005,
};
