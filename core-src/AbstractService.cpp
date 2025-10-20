#include "AbstractService.h"
#include "ActorContext.h"

namespace uranus {
    AbstractService::AbstractService() {
    }

    AbstractService::~AbstractService() {
    }

    void AbstractService::SendToService(const int64_t target, Message *msg) const {
        GetActorContext()->SendToService(target, msg);
    }

    void AbstractService::SendToService(const std::string &target, Message *msg) const {
        GetActorContext()->SendToService(target, msg);
    }

    void AbstractService::SendToPlayer(const int64_t pid, Message *msg) const {
        GetActorContext()->SendToPlayer(pid, msg);
    }

    void AbstractService::SendToClient(const int64_t pid, Message *msg) const {
        GetActorContext()->SendToClient(pid, msg);
    }
}
