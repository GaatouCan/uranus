#include "BaseService.h"
#include "ActorContext.h"
#include "Envelope.h"

namespace uranus::actor {
    BaseService::BaseService() {
    }

    BaseService::~BaseService() {
    }

    void BaseService::sendToClient(PackageHandle &&pkg) const {
        getContext()->send(Envelope::kToClient, 0, std::move(pkg));
    }

    void BaseService::sendToPlayer(const int64_t pid, PackageHandle &&pkg) const {
        getContext()->send(Envelope::kToPlayer, pid, std::move(pkg));
    }

    void BaseService::sendToService(const std::string &name, PackageHandle &&pkg) const {
        if (const auto sid = getContext()->queryActorId("service", name); sid > 0) {
            getContext()->send(Envelope::kToService, sid, std::move(pkg));
        }
    }
}
