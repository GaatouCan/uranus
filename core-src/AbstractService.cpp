#include "AbstractService.h"
#include "ActorContext.h"

namespace uranus {
    AbstractService::AbstractService()
        : id_(kInvalidServiceID) {
    }

    AbstractService::~AbstractService() {
    }

    void AbstractService::SetServiceID(const int64_t id) {
        id_ = id;
    }

    int64_t AbstractService::GetServiceID() const {
        if (id_ < 0)
            return kInvalidServiceID;
        return id_;
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
