#pragma once

#include "Common.h"

namespace uranus {

    class GameServer;

    class CORE_API ServerModule {

    public:
        ServerModule() = delete;

        explicit ServerModule(GameServer* ser);
        virtual ~ServerModule();

        DISABLE_COPY_MOVE(ServerModule)

        [[nodiscard]] GameServer *GetGameServer() const;

        [[nodiscard]] virtual const char *GetModuleName() const = 0;

        virtual void Start();
        virtual void Stop();

    private:
        GameServer *const server_;
    };
}