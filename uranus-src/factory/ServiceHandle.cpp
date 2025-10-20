#include "ServiceHandle.h"
#include "ServiceFactory.h"
#include "AbstractService.h"


ServiceHandle::ServiceHandle()
    : service_(nullptr),
      factory_(nullptr) {
}

ServiceHandle::ServiceHandle(AbstractService *service, ServiceFactory *factory, std::string path)
    : service_(service),
      factory_(factory),
      path_(std::move(path)) {
}

ServiceHandle::~ServiceHandle() {
    Release();
}

ServiceHandle::ServiceHandle(ServiceHandle &&rhs) noexcept {
    service_ = rhs.service_;
    factory_ = rhs.factory_;

    rhs.service_ = nullptr;
    rhs.factory_ = nullptr;

    path_ = std::move(rhs.path_);
}

ServiceHandle &ServiceHandle::operator=(ServiceHandle &&rhs) noexcept {
    if (this != &rhs) {
        Release();

        service_ = rhs.service_;
        factory_ = rhs.factory_;

        rhs.service_ = nullptr;
        rhs.factory_ = nullptr;

        path_ = std::move(rhs.path_);
    }
    return *this;
}

const std::string &ServiceHandle::GetPath() const {
    return path_;
}

bool ServiceHandle::IsValid() const {
    return service_ != nullptr && factory_ != nullptr && !path_.empty();
}

AbstractService *ServiceHandle::operator->() const noexcept {
    return service_;
}

AbstractService &ServiceHandle::operator*() const noexcept {
    return *service_;
}

AbstractService *ServiceHandle::Get() const noexcept {
    return service_;
}

bool ServiceHandle::operator==(const ServiceHandle &rhs) const noexcept {
    return service_ == rhs.service_ &&
           factory_ == rhs.factory_ &&
           path_ == rhs.path_;
}

bool ServiceHandle::operator==(nullptr_t) const noexcept {
    return service_ == nullptr;
}

ServiceHandle::operator bool() const noexcept {
    return IsValid();
}

void ServiceHandle::Release() {
    if (!service_)
        return;

    if (factory_ != nullptr && !path_.empty()) {
        factory_->DestroyInstance(service_, path_);
    } else {
        delete service_;
    }

    service_ = nullptr;
    factory_ = nullptr;
    path_.clear();
}
