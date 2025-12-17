#include "ServiceManager.h"
#include "ServiceContext.h"
#include "GameWorld.h"
#include "factory/ServiceFactory.h"

#include <set>
#include <spdlog/spdlog.h>
#include <actor/BaseService.h>

namespace uranus {

    using actor::BaseActor;

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

            const auto ctx = std::make_shared<ServiceContext>(world_.getWorkerIOContext());

            ctx->setId(sid);
            ctx->setServiceManager(this);
            ctx->setUpActor({ser, [path](BaseActor *ptr) {
                if (!ptr)
                    return;

                if (auto *temp = dynamic_cast<BaseService *>(ptr)) {
                    ServiceFactory::instance().destroy(temp, path);
                    return;
                }

                delete ptr;
            }});

            services_.insert_or_assign(sid, ctx);
        }

        for (const auto &[sid, ctx] : services_) {
            ctx->run();
        }
    }

    void ServiceManager::stop() {
        for (const auto &[sid, ctx] : services_) {
            ctx->terminate();
        }
    }

    GameWorld &ServiceManager::getWorld() const {
        return world_;
    }
} // uranus
