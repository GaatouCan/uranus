#include "ServiceContext.h"
#include "AbstractService.h"
#include "Message.h"
#include "PackageNode.h"
#include "PackagePool.h"
#include "ServiceManager.h"
#include "../GameWorld.h"
#include "../gateway/Gateway.h"
#include "../gateway/Connection.h"
#include "../player/PlayerContext.h"
#include "../player/PlayerManager.h"

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

void ServiceContext::SendToService(const int64_t target, Message *msg) {
    if (!handle_.IsValid()) {
        throw std::runtime_error(std::format(
            "{} - ServiceContext[{:p}] - Service Handle is invalid",
            __FUNCTION__, static_cast<const void *>(this)));
    }

    if (msg == nullptr || msg->data == nullptr || target < 0 || target == handle_->GetServiceID()) {
        Package::ReleaseMessage(msg);
        return;
    }

    msg->type |= Message::kToService;

    if (const auto *mgr = GetGameServer()->GetModule<ServiceManager>()) {
        if (const auto ser = mgr->FindService(target)) {
            ser->PushMessage(msg);
            return;
        }
    }

    Package::ReleaseMessage(msg);
}

void ServiceContext::SendToService(const std::string &name, Message *msg) {
    if (!handle_.IsValid()) {
        throw std::runtime_error(std::format(
            "{} - ServiceContext[{:p}] - Service Handle is invalid",
            __FUNCTION__, static_cast<const void *>(this)));
    }

    if (msg == nullptr || msg->data == nullptr || name.empty()) {
        Package::ReleaseMessage(msg);
        return;
    }

    int64_t target = kInvalidServiceID;

    if (const auto *mgr = GetGameServer()->GetModule<ServiceManager>()) {
        target = mgr->FindServiceID(name);
    }

    if (target >= 0 || target != handle_->GetServiceID()) {
        this->SendToService(target, msg);
        return;
    }

    Package::ReleaseMessage(msg);
}

void ServiceContext::SendToPlayer(const int64_t pid, Message *msg) {
    if (!handle_.IsValid()) {
        throw std::runtime_error(std::format(
            "{} - ServiceContext[{:p}] - Service Handle is invalid",
            __FUNCTION__, static_cast<const void *>(this)));
    }

    if (msg == nullptr || msg->data == nullptr || pid <= 0) {
        Package::ReleaseMessage(msg);
        return;
    }

    msg->type |= Message::kToPlayer;

    if (const auto *mgr = GetGameServer()->GetModule<PlayerManager>()) {
        if (const auto plr = mgr->FindPlayer(pid)) {
            plr->PushMessage(msg);
            return;
        }
    }

    Package::ReleaseMessage(msg);
}

void ServiceContext::SendToClient(const int64_t pid, Message *msg) {
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
