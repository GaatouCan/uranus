#pragma once

#include "config.export.h"
#include "LogicConfig.h"

#include <nlohmann/json.hpp>
#include <unordered_map>

namespace uranus {

    using config::LogicConfig;
    using std::unordered_map;

    /**
     * @brief LogicConfig_Appearance
     */
    class CONFIG_API LC_Appearance final : public LogicConfig {

    public:

        struct AvatarData {
            int id;
            int level;
        };

        bool reload(std::string_view dir) override;

        [[nodiscard]] const AvatarData *findAvatarData(int id) const;

    private:
        int loadAvatarConfig(const nlohmann::json &data);

    private:
        unordered_map<int, AvatarData> avatars_;
    };
}