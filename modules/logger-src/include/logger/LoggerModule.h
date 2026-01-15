#pragma once

#include "logger.export.h"
#include <actor/ServerModule.h>

namespace uranus::logger {

    using actor::ServerModule;

    class LOGGER_API LoggerModule final : public ServerModule {

    public:
        LoggerModule();
        ~LoggerModule() override;

        SERVER_MODULE_NAME(LoggerModule)
        DISABLE_COPY_MOVE(LoggerModule)

        void start() override;
        void stop() override;
    };
}