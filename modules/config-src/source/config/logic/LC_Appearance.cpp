#include "logic/LC_Appearance.h"

#include <fstream>

namespace uranus {
    bool LC_Appearance::reload(std::string_view dir) {
        int ret = 0;

        LOAD_LOGIC_CONFIG("avatar.json", loadAvatarConfig)

        return true;
    }

    const LC_Appearance::AvatarData *LC_Appearance::findAvatarData(const int id) const {
        const auto it = avatars_.find(id);
        return it == avatars_.end() ? nullptr : &it->second;
    }

    int LC_Appearance::loadAvatarConfig(const nlohmann::json &data) {
        for (const auto &node : data) {
            AvatarData cfg;

            cfg.id = node["id"];
            cfg.level = node["level"];

            avatars_[cfg.id] = cfg;
        }

        return 0;
    }
}
