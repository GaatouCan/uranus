#include "ServiceContext.h"
#include "AbstractService.h"
#include "Message.h"
#include "PackageNode.h"
#include "PackagePool.h"
#include "../GameWorld.h"

using uranus::network::Package;
using uranus::network::PackageNode;

ServiceContext::ServiceContext(GameWorld *world)
    : ActorContext(world) {
    pool_ = make_shared<PackagePool>();
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

GameWorld *ServiceContext::GetWorld() const {
    return dynamic_cast<GameWorld *>(GetGameServer());
}

Message *ServiceContext::BuildMessage() {
    auto *msg = new Message();

    msg->type = Message::kFromService;
    msg->session = 0;

    auto *pkg = pool_->Acquire();

    msg->data = pkg;
    msg->length = sizeof(Package);

    return msg;
}

int ServiceContext::Initial(DataAsset *data) {
    pool_->Initial(64);
}

int ServiceContext::Start() {

}

void ServiceContext::CleanUp() {

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
