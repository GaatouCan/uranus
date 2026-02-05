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
        SPDLOG_DEBUG("ServiceManager created");
    }

    ServiceManager::~ServiceManager() {
        SPDLOG_DEBUG("ServiceManager destroyed");
    }

    void ServiceManager::start() {
        SERVICE_FACTORY.initial();

        const auto *config = GET_MODULE(&world_, ConfigModule);
        if (!config) {
            SPDLOG_ERROR("Config module is not available");
            exit(-1);
        }

        const auto &cfg = config->getServerConfig();

        for (const auto &item : cfg["server"]["service"]["core"]) {
            const auto filename = item.as<std::string>();
            const auto sid = idAlloc_.allocate();

            auto [ser, path] = SERVICE_FACTORY.create(filename);
            if (!ser) {
                SPDLOG_ERROR("Create service instance failed: {}", filename);
                exit(-1);
            }

            auto handle = ActorHandle(ser, [filename](BaseActor *ptr) {
                if (!ptr)
                    return;

                if (auto *temp = dynamic_cast<BaseService *>(ptr)) {
                    SERVICE_FACTORY.destroy(temp, filename);
                    return;
                }

                delete ptr;
            });

            auto strand = asio::make_strand(world_.getWorkerIOContext());
            const auto ctx = std::make_shared<ServiceContext>(strand, std::move(handle));

            ctx->attr().set("SERVICE_ID", sid);
            ctx->attr().set("LIBRARY_PATH", path.string());
            ctx->setServiceManager(this);

            services_.insert_or_assign(sid, ctx);
            SPDLOG_INFO("Created service[{} - {}]", sid, filename);
        }

        for (const auto &[sid, ctx] : services_) {
            ctx->run(nullptr);

            const auto name = ctx->getService()->getName();
            SPDLOG_INFO("Started service [{} - {}]", sid, name);

            nameToId_[name] = sid;
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

    set<shared_ptr<ServiceContext>> ServiceManager::getServiceSet(const set<int64_t> &sids) const {
        if (!world_.isRunning())
            return {};

        set<shared_ptr<ServiceContext>> result;
        shared_lock lock(mutex_);

        for (const auto sid : sids) {
            if (const auto it = services_.find(sid); it != services_.end()) {
                result.insert(it->second);
            }
        }

        return result;
    }

    ServiceMap ServiceManager::getServiceMap() const {
        if (!world_.isRunning())
            return {};

        ServiceMap result;

        shared_lock lock(cacheMutex_);
        for (const auto &[name, sid] : nameToId_) {
            result[name] = sid;
        }

        return result;
    }

    int64_t ServiceManager::queryServiceId(const std::string &name) const {
        if (!world_.isRunning())
            return -1;

        shared_lock lock(cacheMutex_);
        const auto iter = nameToId_.find(name);
        return iter == nameToId_.end() ? -1 : iter->second;
    }

    void ServiceManager::updateServiceCache() {
        if (!world_.isRunning())
            return;

        unique_lock cacheLock(cacheMutex_);
        nameToId_.clear();

        shared_lock lock(mutex_);
        for (const auto &[key, val] : services_) {
            nameToId_[val->getService()->getName()] = key;
        }
    }
} // uranus
