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
    if (const auto sid = this->FindServiceID(name); sid >= 0) {
        return this->FindService(sid);
    }
    return nullptr;
}
