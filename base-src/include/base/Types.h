#pragma once

#include <chrono>
#include <thread>

namespace uranus {
    using SteadyTimePoint = std::chrono::steady_clock::time_point;
    using SteadyDuration = std::chrono::duration<SteadyTimePoint>;

    using SystemTimePoint = std::chrono::system_clock::time_point;
    using SystemDuration = std::chrono::duration<SystemTimePoint>;

    using ThreadID = std::thread::id;
}