#include "ServiceManager.h"
#include "ServiceContext.h"

namespace uranus {
    ServiceManager::ServiceManager(GameWorld &world)
        : world_(world) {
    }

    ServiceManager::~ServiceManager() {
    }

    void ServiceManager::start() {
    }

    void ServiceManager::stop() {
    }

    GameWorld &ServiceManager::getWorld() const {
        return world_;
    }
} // uranus
