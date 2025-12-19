#include "ConfigModule.h"

#include <cassert>
#include <yaml-cpp/yaml.h>

namespace uranus::config {
    ConfigModule::ConfigModule() {
    }

    ConfigModule::~ConfigModule() {
    }

    void ConfigModule::start() {
        config_ = YAML::LoadFile("/config/server.yaml");

        assert(!config_["server"].IsNull());
    }

    void ConfigModule::stop() {
    }

    const YAML::Node &ConfigModule::getServerConfig() const {
        return config_;
    }
}
