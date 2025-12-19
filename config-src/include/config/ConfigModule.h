#pragma once

#include "config.export.h"

#include <actor/ServerModule.h>
#include <yaml-cpp/node/node.h>

namespace uranus::config {

    using actor::ServerModule;

    class CONFIG_API ConfigModule final : public ServerModule {

    public:
        ConfigModule();
        ~ConfigModule() override;

        SERVER_MODULE_NAME(ConfigModule);

        void start() override;
        void stop() override;

        const YAML::Node &getServerConfig() const;

    private:
        YAML::Node config_;
    };
} // config
// uranus
