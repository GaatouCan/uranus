#include "ServiceContext.h"
#include "AbstractService.h"
#include "../GameWorld.h"

ServiceContext::ServiceContext(GameWorld *world)
    : ActorContext(world) {
}

ServiceContext::~ServiceContext() {
}

AbstractActor *ServiceContext::GetActor() const {
    if (!handle_.IsValid()) {
        throw std::runtime_error(std::format(
            "{} - ServiceContext[{:p}] - Service Handle is invalid",
            __FUNCTION__, static_cast<const void *>(this)));
    }

    return handle_.Get();
}

Message *ServiceContext::BuildMessage() {
}

void ServiceContext::SendToService(int64_t target, Message *msg) {
}

void ServiceContext::SendToService(const std::string &name, Message *msg) {
}

void ServiceContext::SendToPlayer(int64_t pid, Message *msg) {
}

void ServiceContext::SendToClient(int64_t pid, Message *msg) {
}

void ServiceContext::PushMessage(Message *msg) {
}

void ServiceContext::SetUpService(ServiceHandle &&handle) {
    handle_ = std::move(handle);
}
