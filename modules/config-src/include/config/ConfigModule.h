#pragma once

#include "config.export.h"
#include "LogicConfig.h"
#include "LogicConfigType.h"

#include <unordered_map>
#include <memory>
#include <actor/ServerModule.h>
#include <yaml-cpp/node/node.h>

namespace uranus::config {

    using actor::ServerModule;
    using std::unordered_map;
    using std::unique_ptr;
    using std::make_unique;

    class CONFIG_API ConfigModule final : public ServerModule {

    public:
        ConfigModule();
        ~ConfigModule() override;

        DISABLE_COPY_MOVE(ConfigModule)
        SERVER_MODULE_NAME(ConfigModule);

        void start() override;
        void stop() override;

        [[nodiscard]] const YAML::Node& getServerConfig() const;

        template<class T>
        requires std::derived_from<T, LogicConfig>
        [[nodiscard]] const T *find(LogicConfigType ty) const;

    private:
        void registerLogicConfig();

    private:
        YAML::Node config_;

        unordered_map<LogicConfigType, std::string> pathMap_;
        unordered_map<LogicConfigType, unique_ptr<LogicConfig>> logicMap_;
    };

    template<class T>
    requires std::derived_from<T, LogicConfig>
    const T *ConfigModule::find(const LogicConfigType ty) const {
        const auto it = logicMap_.find(ty);

        if (it == logicMap_.end())
            return nullptr;

        const auto *ptr = it->second.get();
        return dynamic_cast<const T *>(ptr);
    }
}
