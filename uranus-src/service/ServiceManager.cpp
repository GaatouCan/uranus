#include "ServiceManager.h"
#include "ServiceContext.h"
#include "factory/ServiceFactory.h"

#include <set>
#include <spdlog/spdlog.h>


namespace uranus {
    ServiceManager::ServiceManager(GameWorld &world)
        : world_(world) {
    }

    ServiceManager::~ServiceManager() {
    }

    void ServiceManager::start() {
        ServiceFactory::instance().initial();

        // FIXME: Read from config
        std::set<std::string> services;

        for (const auto &path : services) {
            const auto sid = idAlloc_.allocate();

            auto *ser = ServiceFactory::instance().create(path);
            if (!ser) {
                // TODO: log
                exit(-1);
            }
        }
    }

    void ServiceManager::stop() {
    }

    GameWorld &ServiceManager::getWorld() const {
        return world_;
    }
} // uranus
