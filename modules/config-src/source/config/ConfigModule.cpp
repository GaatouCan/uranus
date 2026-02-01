#include "ConfigModule.h"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>
#include <cassert>


namespace uranus::config {

    static constexpr auto kServerConfigFile = "config/server.yaml";

    ConfigModule::ConfigModule() {
        SPDLOG_DEBUG("ConfigModule created");
        SPDLOG_INFO("Using configuration file: {}", kServerConfigFile);
        config_ = YAML::LoadFile(kServerConfigFile);

        assert(!config_["server"].IsNull());

        assert(!config_["server"]["network"].IsNull());
        assert(!config_["server"]["network"]["port"].IsNull());
        assert(!config_["server"]["network"]["threads"].IsNull());

        assert(!config_["server"]["worker"].IsNull());
        assert(!config_["server"]["worker"]["threads"].IsNull());

        assert(!config_["server"]["service"].IsNull());
        assert(config_["server"]["service"]["core"] && config_["server"]["service"]["core"].IsSequence());
        // assert(config_["server"]["service"]["extend"] && config_["server"]["service"]["extend"].IsSequence());

        SPDLOG_INFO("Load configuration file success");
    }

    ConfigModule::~ConfigModule() {
        SPDLOG_DEBUG("ConfigModule destroyed");
    }

    void ConfigModule::start() {
        registerLogicConfig();

        for (auto &[ty, val] : logicMap_) {
            const auto it = pathMap_.find(ty);
            if (it == pathMap_.end()) {
                SPDLOG_ERROR("Failed to find config path: {}", -(static_cast<int>(ty)));
                std::abort();
            }

            const auto &path = it->second;

            if (!val->reload(path)) {
                SPDLOG_ERROR("Failed to load config file: {}", path);
                std::abort();
            }
        }
    }

    void ConfigModule::stop() {
    }

    const YAML::Node &ConfigModule::getServerConfig() const {
        return config_;
    }
}
