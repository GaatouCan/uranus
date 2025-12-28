#pragma once

#include <cstdint>

namespace uranus::protocol {
    static constexpr uint32_t kClientLoginRequest = 1001;
    static constexpr uint32_t kServerLoginResponse = 1002;
    static constexpr uint32_t kLoginFailed = 1003;
    static constexpr uint32_t kLoginRepeated = 1004;
    static constexpr uint32_t kHeartbeat = 1005;
}