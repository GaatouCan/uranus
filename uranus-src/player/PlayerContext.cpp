#include "PlayerContext.h"
#include "AbstractPlayer.h"
#include "Message.h"
#include "Package.h"
#include "PackageNode.h"

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
}

void PlayerContext::SendToClient(int64_t pid, Message *msg) {
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

    auto node = std::make_unique<PackageNode>();
    node->SetMessage(msg);

    this->PushNode(std::move(node));
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
