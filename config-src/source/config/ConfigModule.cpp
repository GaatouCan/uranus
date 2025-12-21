#include "ConfigModule.h"

#include <cassert>
#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>

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

        SPDLOG_INFO("Load configuration file success");
    }

    void ConfigModule::stop() {
    }

    const YAML::Node &ConfigModule::getServerConfig() const {
        return config_;
    }
}
