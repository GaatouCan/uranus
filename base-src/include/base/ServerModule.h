#pragma once

#include "base.export.h"
#include "noncopy.h"

namespace uranus {

    class ServerBootstrap;

    class BASE_API ServerModule {

    public:
        ServerModule() = delete;

        explicit ServerModule(ServerBootstrap &server);
        virtual ~ServerModule();

        DISABLE_COPY_MOVE(ServerModule)

        virtual const char *getModuleName() = 0;

        [[nodiscard]] ServerBootstrap &getServer() const;

        virtual void start() = 0;
        virtual void stop() = 0;

    private:
        ServerBootstrap &server_;
    };
}