#pragma once

#include "config.export.h"
#include "LogicConfig.h"

namespace uranus {

    using config::LogicConfig;

    class CONFIG_API LC_Avatar final : public LogicConfig {

    public:
        bool reload(std::string_view dir) override;
    };
}