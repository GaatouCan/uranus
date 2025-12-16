#include "ServiceContext.h"
#include "ServiceManager.h"

namespace uranus {
    ServiceContext::ServiceContext(asio::io_context &ctx)
        : ActorContext(ctx),
          manager_(nullptr) {
    }

    ServiceContext::~ServiceContext() {
    }

    void ServiceContext::send(int ty, uint32_t target, actor::PackageHandle &&pkg) {
    }

    ServiceManager *ServiceContext::getServiceManager() const {
        return manager_;
    }

    GameWorld *ServiceContext::getWorld() const {
        if (manager_) {
            return &manager_->getWorld();
        }
        return nullptr;
    }

    void ServiceContext::setServiceManager(ServiceManager *mgr) {
        manager_ = mgr;
    }
}
