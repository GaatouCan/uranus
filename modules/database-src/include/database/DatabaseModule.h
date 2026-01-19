#pragma once

#include "database.export.h"

#include <actor/ServerModule.h>

namespace uranus::database {

    using actor::ServerModule;

    class DATABASE_API DatabaseModule final : public ServerModule {

    public:
        DatabaseModule();
        ~DatabaseModule() override;

        SERVER_MODULE_NAME(DatabaseModule);
        DISABLE_COPY_MOVE(DatabaseModule)

        void start() override;
        void stop() override;
    };
}