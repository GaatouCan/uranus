#include "PlayerContext.h"
#include "AbstractPlayer.h"
#include "Message.h"
#include "Package.h"
#include "PackageNode.h"
#include "../gateway/Connection.h"
#include "../gateway/Gateway.h"

using uranus::network::Package;
using uranus::network::PackageNode;


PlayerContext::PlayerContext(GameServer *ser)
    : ActorContext(ser) {
}

PlayerContext::~PlayerContext() {
}

AbstractActor *PlayerContext::GetActor() const {
    if (!handle_.IsValid()) {
        throw std::runtime_error(std::format(
            "{} - PlayerContext[{:p}] - Player Handle is invalid",
            __FUNCTION__, static_cast<const void *>(this)));
    }

    return handle_.Get();
}

int PlayerContext::Initial(DataAsset *data) {
    if (!handle_.IsValid()) {
        throw std::runtime_error(std::format(
            "{} - PlayerContext[{:p}] - Player Handle is invalid",
            __FUNCTION__, static_cast<const void *>(this)));
    }

    const auto ret = handle_->Initial(data);
    return ret;
}

int PlayerContext::Start() {
    if (!handle_.IsValid()) {
        throw std::runtime_error(std::format(
            "{} - PlayerContext[{:p}] - Player Handle is invalid",
            __FUNCTION__, static_cast<const void *>(this)));
    }

    const auto ret = handle_->Start();
    if (ret != 1) {
        return ret;
    }

    co_spawn(GetIOContext(), [self = shared_from_this(), this] mutable -> awaitable<void> {
        co_await this->Process();
        this->CleanUp();
    }, detached);


    return 1;
}

void PlayerContext::SendToService(int64_t target, Message *msg) {
}

void PlayerContext::SendToService(const std::string &name, Message *msg) {
}

void PlayerContext::SendToPlayer(int64_t pid, Message *msg) {
    // Player can not send to other player directly,
    // so this method will do nothing but only release the message

    if (msg != nullptr && msg->data != nullptr) {
        auto *pkg = static_cast<Package *>(msg->data);
        pkg->Recycle();
        delete msg;
    }
}

void PlayerContext::SendToClient(int64_t pid, Message *msg) {
    if (!handle_.IsValid()) {
        throw std::runtime_error(std::format(
            "{} - PlayerContext[{:p}] - Player Handle is invalid",
            __FUNCTION__, static_cast<const void *>(this)));
    }

    if (msg == nullptr || msg->data == nullptr)
        return;

    if (pid < 0 || pid != handle_->GetPlayerID()) {
        auto *pkg = static_cast<Package *>(msg->data);
        pkg->Recycle();
        delete msg;

        return;
    }

    auto *gateway = GetGameServer()->GetModule<Gateway>();
    if (gateway != nullptr) {

        // FIXME: Use Player ID To Find Connection
        if (const auto conn = gateway->FindConnection("")) {
            conn->SendToClient(msg);
            return;
        }
    }

    auto *pkg = static_cast<Package *>(msg->data);
    pkg->Recycle();
    delete msg;
}

void PlayerContext::PushMessage(Message *msg) {
    if (msg == nullptr || msg->data == nullptr)
        return;

    if (IsChannelClosed()) {
        auto *pkg = static_cast<Package *>(msg->data);
        pkg->Recycle();
        delete msg;
        return;
    }

    auto *node = new PackageNode();
    node->SetMessage(msg);

    this->PushNode(node);
}

void PlayerContext::CleanUp() {
    if (handle_) {
        handle_->Stop();
    }
    handle_.Release();
}

void PlayerContext::SetUpPlayer(PlayerHandle &&handle) {
    handle_ = std::move(handle);
    this->SetUpActor();
}
