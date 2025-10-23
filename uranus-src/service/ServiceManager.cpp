#include "ServiceManager.h"
#include "../factory/ServiceFactory.h"
#include "../GameWorld.h"
#include "ConfigModule.h"
#include "GameServer.h"
#include "ServiceContext.h"
#include "AbstractService.h"

#include <spdlog/spdlog.h>


using uranus::config::ConfigModule;

ServiceManager::ServiceManager(GameServer *ser)
    : ServerModule(ser),
      id_alloc_() {
    factory_ = make_unique<ServiceFactory>();
}

ServiceManager::~ServiceManager() {
}

void ServiceManager::Start() {
    const auto &cfg = GetGameServer()->GetModule<ConfigModule>()->GetServerConfig();

    for (const auto &item : cfg["service"]["core"]) {
        const auto path = item["path"].as<std::string>();

        auto service = factory_->CreateInstance(path);

        if (!service.IsValid()) {
            SPDLOG_ERROR("Could not create service, path[{}]", path);
            continue;
        }

        auto sid = id_alloc_.Allocate();
        while (services_.contains(sid)) {
            sid = id_alloc_.Allocate();
        }

        const auto ctx = make_shared<ServiceContext>(dynamic_cast<GameWorld *>(GetGameServer()));

        service->SetServiceID(sid);
        ctx->SetUpService(std::move(service));

        if (const auto ret = ctx->Initial(nullptr); ret != 1) {
            SPDLOG_ERROR("Initial service[{}] failed, ret: {}", path, ret);
            ctx->Stop();
            id_alloc_.Recycle(sid);
            continue;
        }

        SPDLOG_INFO("Service [{}] initial success", path);
        services_[sid] = ctx;
    }

    for (const auto &ctx : services_ | std::views::values) {
        SPDLOG_INFO("Start service[{}]", ctx->GetService()->GetServiceName());
        ctx->Start();
    }
}

void ServiceManager::Stop() {
    for (const auto &ctx : services_ | std::views::values) {
        SPDLOG_INFO("Stop service[{}]", ctx->GetService()->GetServiceName());
        ctx->Stop();
    }
}

int64_t ServiceManager::FindServiceID(const std::string &name) const {
    shared_lock lock(name_mutex_);
    const auto iter = name_to_id_.find(name);
    return iter != name_to_id_.end() ? iter->second : kInvalidServiceID;
}

shared_ptr<ServiceContext> ServiceManager::FindService(const int64_t sid) const {
    shared_lock lock(mutex_);
    const auto iter = services_.find(sid);
    return iter != services_.end() ? iter->second : nullptr;
}

shared_ptr<ServiceContext> ServiceManager::FindService(const std::string &name) const {
    if (const auto sid = FindServiceID(name); sid >= 0) {
        return FindService(sid);
    }
    return nullptr;
}
