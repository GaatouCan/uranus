#include "ServiceFactory.h"

namespace uranus {
    ServiceFactory::ServiceFactory() {
    }

    ServiceFactory::~ServiceFactory() {
        services_.clear();
    }

    ServiceFactory &ServiceFactory::instance() {
        static ServiceFactory _inst;
        return _inst;
    }

    void ServiceFactory::initial() {
    }

    BaseService *ServiceFactory::create(const std::string &path) {
    }

    void ServiceFactory::destroy(BaseService *ptr, const std::string &path) {
    }
} // uranus