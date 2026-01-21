#pragma once

#include "database.export.h"

#include <actor/ServerModule.h>
#include <functional>
#include <vector>
#include <string>

namespace uranus::database {

    using actor::ServerModule;

    class DATABASE_API DatabaseModule final : public ServerModule {

        // Use bson
        using ResultCallback = std::function<void(bool, const std::vector<uint8_t> &)>;

    public:
        DatabaseModule();
        ~DatabaseModule() override;

        SERVER_MODULE_NAME(DatabaseModule);
        DISABLE_COPY_MOVE(DatabaseModule)

        void start() override;
        void stop() override;

        void query(const std::string &table, const std::string &cond, const ResultCallback &cb);
    };
}