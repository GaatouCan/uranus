#include "ConfigModule.h"
#include "base/Utils.h"

#include <cassert>
#include <fstream>
#include <spdlog/spdlog.h>

inline constexpr auto kServerConfigFile = "config/config.yaml";
inline constexpr auto kConfigJsonPath = "config/json";

namespace uranus::config {
    ConfigModule::ConfigModule(GameServer *ser)
        : ServerModule(ser) {

        SPDLOG_INFO("Use server configuration file: {}", kServerConfigFile);
        config_ = YAML::LoadFile(kServerConfigFile);

        assert(!config_.IsNull());
        SPDLOG_INFO("Checking configuration file");

        assert(!config_["server"].IsNull());
        assert(!config_["server"]["port"].IsNull());

        assert(!config_["server"]["network"].IsNull());
        assert(!config_["server"]["network"]["worker"].IsNull());
        assert(!config_["server"]["network"]["expiration"].IsNull());
        assert(!config_["server"]["network"]["outputBuffer"].IsNull());
        assert(!config_["server"]["network"]["recycler"].IsNull());
        assert(!config_["server"]["network"]["recycler"]["initialCapacity"].IsNull());
        assert(!config_["server"]["network"]["recycler"]["minimumCapacity"].IsNull());
        assert(!config_["server"]["network"]["recycler"]["halfCollect"].IsNull());
        assert(!config_["server"]["network"]["recycler"]["fullCollect"].IsNull());
        assert(!config_["server"]["network"]["recycler"]["collectThreshold"].IsNull());
        assert(!config_["server"]["network"]["recycler"]["collectRate"].IsNull());

        assert(!config_["server"]["actor"].IsNull());
        assert(!config_["server"]["actor"]["worker"].IsNull());

        assert(!config_["service"].IsNull());
        assert(!config_["service"]["core"].IsNull());
        assert(!config_["service"]["extend"].IsNull());
        assert(!config_["service"]["queueBuffer"].IsNull());
        assert(!config_["service"]["recycler"].IsNull());
        assert(!config_["service"]["recycler"]["InitialCapacity"].IsNull());
        assert(!config_["service"]["recycler"]["minimumCapacity"].IsNull());
        assert(!config_["service"]["recycler"]["halfCollect"].IsNull());
        assert(!config_["service"]["recycler"]["fullCollect"].IsNull());
        assert(!config_["service"]["recycler"]["collectThreshold"].IsNull());
        assert(!config_["service"]["recycler"]["collectRate"].IsNull());

        assert(!config_["player"].IsNull());
        assert(!config_["player"]["queueBuffer"].IsNull());
        assert(!config_["player"]["recycler"].IsNull());
        assert(!config_["player"]["recycler"]["InitialCapacity"].IsNull());
        assert(!config_["player"]["recycler"]["minimumCapacity"].IsNull());
        assert(!config_["player"]["recycler"]["halfCollect"].IsNull());
        assert(!config_["player"]["recycler"]["fullCollect"].IsNull());
        assert(!config_["player"]["recycler"]["collectThreshold"].IsNull());
        assert(!config_["player"]["recycler"]["collectRate"].IsNull());

        SPDLOG_INFO("Load server configuration successfully");

        const std::string json_path = kConfigJsonPath;
        utils::TraverseFolder(json_path, [this, json_path](const std::filesystem::directory_entry &entry) {
            if (entry.path().extension().string() == ".json") {
                std::ifstream fs(entry.path());

                auto filepath = entry.path().string();
                filepath = filepath.substr(strlen(json_path.c_str()) + 1, filepath.length() - 6 - strlen(json_path.c_str()));

        #ifdef WIN32
                    filepath = utils::StringReplace(filepath, '\\', '.');
        #elifdef __linux__
                    filepath = utils::StringReplace(filepath, '/', '.');
        #else
                    filepath = utils::StringReplace(filepath, '/', '.');
        #endif

                    config_map_[filepath] = nlohmann::json::parse(fs);
                    SPDLOG_INFO("\tLoaded {}.", filepath);
                }
            });

        SPDLOG_INFO("JSON files loaded successfully");
    }

    ConfigModule::~ConfigModule() {
    }

    const nlohmann::json *ConfigModule::FindConfig(const std::string &path) const {
        const auto iter = config_map_.find(path);
        if (iter == config_map_.end())
            return nullptr;
        return &iter->second;
    }
}
