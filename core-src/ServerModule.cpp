#include "ServerModule.h"

namespace uranus {
    ServerModule::ServerModule(GameServer *ser)
        : server_(ser) {
    }

    ServerModule::~ServerModule() {
    }

    GameServer *ServerModule::GetGameServer() const {
        return server_;
    }

    void ServerModule::Start() {
    }

    void ServerModule::Stop() {
    }
}