#include "ServiceContext.h"
#include "AbstractService.h"
#include "Message.h"
#include "PackageNode.h"
#include "PackagePool.h"
#include "../GameWorld.h"
#include "../gateway/Gateway.h"
#include "../gateway/Connection.h"

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
    if (!handle_.IsValid()) {
        throw std::runtime_error(std::format(
            "{} - ServiceContext[{:p}] - Service Handle is invalid",
            __FUNCTION__, static_cast<const void *>(this)));
    }

    pool_->Initial(64);

    const auto ret = handle_->Initial(data);
    return ret;
}

int ServiceContext::Start() {
    if (!handle_.IsValid()) {
        throw std::runtime_error(std::format(
            "{} - ServiceContext[{:p}] - Service Handle is invalid",
            __FUNCTION__, static_cast<const void *>(this)));
    }

    const auto ret = handle_->Start();
    if (ret != 1) {
        return ret;
    }

    co_spawn(GetIOContext(), [self = shared_from_this(), this]() mutable -> awaitable<void> {
        co_await this->Process();
        this->CleanUp();
    }, detached);

    return 1;
}

void ServiceContext::CleanUp() {
    if (handle_.IsValid()) {
        handle_->Stop();
    }
    handle_.Release();
}

void ServiceContext::SendToService(int64_t target, Message *msg) {
}

void ServiceContext::SendToService(const std::string &name, Message *msg) {
}

void ServiceContext::SendToPlayer(int64_t pid, Message *msg) {
}

void ServiceContext::SendToClient(int64_t pid, Message *msg) {
    if (!handle_.IsValid()) {
        throw std::runtime_error(std::format(
            "{} - ServiceContext[{:p}] - Service Handle is invalid",
            __FUNCTION__, static_cast<const void *>(this)));
    }

    if (msg == nullptr || msg->data == nullptr || pid <= 0) {
        Package::ReleaseMessage(msg);
        return;
    }

    msg->type |= Message::kToClient;

    if (const auto *gateway = GetGameServer()->GetModule<Gateway>()) {
        if (const auto conn = gateway->FindConnection(pid)) {
            conn->SendToClient(msg);
            return;
        }
    }

    Package::ReleaseMessage(msg);
}

void ServiceContext::PushMessage(Message *msg) {
    if (msg == nullptr || msg->data == nullptr || IsChannelClosed()) {
        Package::ReleaseMessage(msg);
        return;
    }

    auto *node = new PackageNode();
    node->SetMessage(msg);

    this->PushNode(node);
}

void ServiceContext::SetUpService(ServiceHandle &&handle) {
    handle_ = std::move(handle);
    SetUpActor();
}
