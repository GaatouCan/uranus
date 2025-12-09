#include "ServerModule.h"

namespace uranus {
    ServerModule::ServerModule(ServerBootstrap &server)
        : server_(server) {
    }

    ServerModule::~ServerModule() {
    }

    ServerBootstrap &ServerModule::getServer() const {
        return server_;
    }
}
