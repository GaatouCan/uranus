#include "ConfigModule.h"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>
#include <cassert>

namespace uranus::config {
    ConfigModule::ConfigModule() {
    }

    ConfigModule::~ConfigModule() {
    }

    void ConfigModule::start() {
        SPDLOG_INFO("Using configuration file: config/server.yaml");
        config_ = YAML::LoadFile("config/server.yaml");

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

    void ConfigModule::stop() {
    }

    const YAML::Node &ConfigModule::getServerConfig() const {
        return config_;
    }

    // const LogicConfig *ConfigModule::find(const std::string &path) const {
    //     const auto it = logicMap_.find(path);
    //     if (it == logicMap_.end())
    //         return nullptr;
    //
    //     return it->second.get();
    // }
}
