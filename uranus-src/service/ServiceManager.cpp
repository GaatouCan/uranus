#include "ServiceManager.h"
#include "../factory/ServiceFactory.h"

ServiceManager::ServiceManager(GameServer *ser)
    : ServerModule(ser) {
    factory_ = make_unique<ServiceFactory>();
}

ServiceManager::~ServiceManager() {
}

void ServiceManager::Start() {
    factory_->LoadService();
}

void ServiceManager::Stop() {

}

shared_ptr<ServiceContext> ServiceManager::FindService(const int64_t sid) const {
    shared_lock lock(mutex_);
    const auto iter = services_.find(sid);
    return iter != services_.end() ? iter->second : nullptr;
}
