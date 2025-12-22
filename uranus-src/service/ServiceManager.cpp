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
                SPDLOG_ERROR("Create service instance failed: {}", path);
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
            SPDLOG_INFO("Created service[{} - {}]", sid, path);
        }

        for (const auto &[sid, ctx] : services_) {
            ctx->run();
            SPDLOG_INFO("Started service [{} - {}]", sid, ctx->getService()->getName());
        }
    }

    void ServiceManager::stop() {
        for (const auto &[sid, ctx] : services_) {
            ctx->terminate();
            SPDLOG_INFO("Stopped service [{} - {}]", sid, ctx->getService()->getName());
        }
    }

    GameWorld &ServiceManager::getWorld() const {
        return world_;
    }

    shared_ptr<ServiceContext> ServiceManager::find(const uint32_t sid) const {
        if (!world_.isRunning())
            return nullptr;

        shared_lock lock(mutex_);
        const auto iter = services_.find(sid);
        return iter == services_.end() ? nullptr : iter->second;
    }

    map<std::string, uint32_t> ServiceManager::getServiceList() const {
        if (!world_.isRunning())
            return {};

        shared_lock lock(mutex_);

        map<std::string, uint32_t> res;
        for (const auto &[key, val] : services_) {
            res[val->getService()->getName()] = key;
        }

        return res;
    }
} // uranus
