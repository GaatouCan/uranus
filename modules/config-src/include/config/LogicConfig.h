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

#define LOAD_LOGIC_CONFIG(filename, func)                           \
{                                                                   \
    std::ifstream is(std::string(dir) + "/" + filename);            \
    const nlohmann::json data = nlohmann::json::parse(is);          \
    if (const int ret = this->func(data); ret != 0) {               \
        SPDLOG_ERROR("failed to load {}, ret = {}", filename, ret); \
        std::abort();                                               \
    }                                                               \
}

}