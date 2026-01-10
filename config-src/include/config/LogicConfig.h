#pragma once

#include <base/noncopy.h>
#include <string_view>

namespace uranus::config {

    class LogicConfig {

    public:
        LogicConfig() = default;
        virtual ~LogicConfig() = default;

        DISABLE_COPY_MOVE(LogicConfig)

        virtual bool reload(std::string_view dir) = 0;
    };
}