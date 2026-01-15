#pragma once

#include "logger.export.h"

#include <actor/ServerModule.h>
#include <spdlog/spdlog.h>

namespace uranus::logger {

    using actor::ServerModule;
    using std::shared_ptr;

    class LOGGER_API LoggerModule final : public ServerModule {

    public:
        LoggerModule();
        ~LoggerModule() override;

        SERVER_MODULE_NAME(LoggerModule)
        DISABLE_COPY_MOVE(LoggerModule)

        void start() override;
        void stop() override;

        shared_ptr<spdlog::logger> createLogger(std::string_view name, std::string_view path);
    };
}