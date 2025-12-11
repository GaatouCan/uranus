#include "ServerModule.h"

namespace uranus::actor {
    ServerModule::ServerModule(GameWorld &world)
        : world_(world) {
    }

    ServerModule::~ServerModule() {
    }

    GameWorld &ServerModule::getWorld() const {
        return world_;
    }
}
