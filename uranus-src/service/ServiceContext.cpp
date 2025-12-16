#include "ServiceContext.h"

namespace uranus {
    ServiceContext::ServiceContext(asio::io_context &ctx)
        : ActorContext(ctx) {
    }

    ServiceContext::~ServiceContext() {
    }

    void ServiceContext::send(int ty, uint32_t target, actor::PackageHandle &&pkg) {
    }
}
