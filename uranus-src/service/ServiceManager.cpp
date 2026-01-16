#include "ServiceManager.h"
#include "ServiceContext.h"
#include "GameWorld.h"
#include "factory/ServiceFactory.h"

#include <actor/BaseService.h>
#include <config/ConfigModule.h>
#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>


namespace uranus {

    using actor::BaseActor;
    using config::ConfigModule;

    ServiceManager::ServiceManager(GameWorld &world)
        : world_(world) {
    }

    ServiceManager::~ServiceManager() {
    }

    void ServiceManager::start() {
        ServiceFactory::instance().initial();

        const auto *config = GET_MODULE(&world_, ConfigModule);
        if (!config) {
            SPDLOG_ERROR("Config module is not available");
            exit(-1);
        }

        const auto &cfg = config->getServerConfig();

        for (const auto &item : cfg["server"]["service"]["core"]) {
            const auto filename = item.as<std::string>();
            const auto sid = idAlloc_.allocate();

            auto [ser, path] = ServiceFactory::instance().create(filename);
            if (!ser) {
                SPDLOG_ERROR("Create service instance failed: {}", filename);
                exit(-1);
            }

            auto handle = ActorHandle(ser, [filename](BaseActor *ptr) {
                if (!ptr)
                    return;

                if (auto *temp = dynamic_cast<BaseService *>(ptr)) {
                    ServiceFactory::instance().destroy(temp, filename);
                    return;
                }

                delete ptr;
            });

            const auto ctx = std::make_shared<ServiceContext>(world_.getWorkerIOContext(), std::move(handle));

            ctx->setServiceManager(this);
            ctx->setServiceId(sid);
            ctx->attr().set("LIBRARY_PATH", path.string());

            services_.insert_or_assign(sid, ctx);
            SPDLOG_INFO("Created service[{} - {}]", sid, filename);
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

    shared_ptr<ServiceContext> ServiceManager::find(const int64_t sid) const {
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
