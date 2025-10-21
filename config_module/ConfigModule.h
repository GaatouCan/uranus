#pragma once

#include "ServerModule.h"

#include <yaml-cpp/yaml.h>
#include <nlohmann/json.hpp>
#include <unordered_map>


namespace uranus::config {

    class CONFIG_API ConfigModule final : public ServerModule {

    public:
        explicit ConfigModule(GameServer *ser);
        ~ConfigModule() override;

        [[nodiscard]] constexpr const char *GetModuleName() const override {
            return "Config Module";
        }

        [[nodiscard]] const YAML::Node &GetServerConfig() const {
            return config_;
        }

        [[nodiscard]] const nlohmann::json *FindConfig(const std::string &path) const;

    private:
        YAML::Node config_;
        std::unordered_map<std::string, nlohmann::json> config_map_;
    };
}