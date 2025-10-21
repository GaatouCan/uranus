#pragma once

#include "Common.h"

#include <vector>
#include <map>
#include <nlohmann/json.hpp>

namespace uranus::config {

    using std::vector;
    using std::map;

    class CONFIG_API LogicConfig {

    public:
        LogicConfig() = default;
        virtual ~LogicConfig() = default;

        [[nodiscard]] virtual vector<std::string> InitialList() const = 0;
        virtual int LoadConfig(const map<std::string, nlohmann::json *> &data) = 0;
    };
}