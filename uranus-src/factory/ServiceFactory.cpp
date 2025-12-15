#include "ServiceFactory.h"

namespace uranus {
    ServiceFactory::ServiceFactory() {
    }

    ServiceFactory::~ServiceFactory() {
    }

    ServiceFactory &ServiceFactory::instance() {
        static ServiceFactory _inst;
        return _inst;
    }
} // uranus