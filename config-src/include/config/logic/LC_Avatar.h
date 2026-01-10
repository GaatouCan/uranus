#pragma once

#include "config.export.h"
#include "LogicConfig.h"

namespace uranus {

    using config::LogicConfig;

    /**
     * @brief LogicConfig_Avatar
     */
    class CONFIG_API LC_Avatar final : public LogicConfig {

    public:
        bool reload(std::string_view dir) override;
    };
}