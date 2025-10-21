#include "PlayerContext.h"
#include "AbstractPlayer.h"
#include "Message.h"
#include "Package.h"
#include "PackageNode.h"
#include "PackagePool.h"
#include "ConfigModule.h"
#include "../GameWorld.h"
#include "../gateway/Connection.h"
#include "../gateway/Gateway.h"
#include "../service/ServiceContext.h"
#include "../service/ServiceManager.h"


using uranus::network::Package;
using uranus::network::PackageNode;
using uranus::config::ConfigModule;


PlayerContext::PlayerContext(GameWorld *world)
    : ActorContext(world) {

    const auto &cfg = GetGameServer()->GetModule<ConfigModule>()->GetServerConfig();

    const auto outputSize = cfg["player"]["queueBuffer"].as<int>();

    const auto initialCapacity      = cfg["player"]["recycler"]["initialCapacity"].as<int>();
    const auto minimumCapacity      = cfg["player"]["recycler"]["minimumCapacity"].as<int>();
    const auto halfCollect          = cfg["player"]["recycler"]["halfCollect"].as<int>();
    const auto fullCollect          = cfg["player"]["recycler"]["fullCollect"].as<int>();
    const auto collectThreshold = cfg["player"]["recycler"]["collectThreshold"].as<double>();
    const auto collectRate      = cfg["player"]["recycler"]["collectRate"].as<double>();

    channel_ = make_unique<ActorChannel>(GetIOContext(), outputSize);

    pool_ = make_shared<PackagePool>();

    pool_->SetHalfCollect(halfCollect);
    pool_->SetFullCollect(fullCollect);
    pool_->SetMinimumCapacity(minimumCapacity);
    pool_->SetCollectThreshold(collectThreshold);
    pool_->SetCollectRate(collectRate);
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

GameWorld *PlayerContext::GetWorld() const {
    return dynamic_cast<GameWorld *>(GetGameServer());
}

int PlayerContext::Initial(DataAsset *data) {
    if (!handle_.IsValid()) {
        throw std::runtime_error(std::format(
            "{} - PlayerContext[{:p}] - Player Handle is invalid",
            __FUNCTION__, static_cast<const void *>(this)));
    }

    pool_->Initial(64);

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

Message *PlayerContext::BuildMessage() {
    auto *msg = new Message();

    msg->type = Message::kFromPlayer;
    msg->session = 0;

    auto *pkg = pool_->Acquire();

    msg->data = pkg;
    msg->length = sizeof(Package);

    return msg;
}

void PlayerContext::SendToService(const int64_t target, Message *msg) {
    if (!handle_.IsValid()) {
        throw std::runtime_error(std::format(
            "{} - PlayerContext[{:p}] - Player Handle is invalid",
            __FUNCTION__, static_cast<const void *>(this)));
    }

    if (msg == nullptr || msg->data == nullptr || target < 0) {
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

void PlayerContext::SendToService(const std::string &name, Message *msg) {
    if (!handle_.IsValid()) {
        throw std::runtime_error(std::format(
            "{} - PlayerContext[{:p}] - Player Handle is invalid",
            __FUNCTION__, static_cast<const void *>(this)));
    }

    if (msg == nullptr || msg->data == nullptr || name.empty()) {
        Package::ReleaseMessage(msg);
        return;
    }

    msg->type |= Message::kToService;

    if (const auto *mgr = GetGameServer()->GetModule<ServiceManager>()) {
        if (const auto ser = mgr->FindService(name)) {
            ser->PushMessage(msg);
            return;
        }
    }

    Package::ReleaseMessage(msg);
}

void PlayerContext::SendToPlayer(int64_t pid, Message *msg) {
    // Player can not send to other player directly,
    // so this method will do nothing but only release the message
    Package::ReleaseMessage(msg);
}

void PlayerContext::SendToClient(const int64_t pid, Message *msg) {
    if (!handle_.IsValid()) {
        throw std::runtime_error(std::format(
            "{} - PlayerContext[{:p}] - Player Handle is invalid",
            __FUNCTION__, static_cast<const void *>(this)));
    }

    if (msg == nullptr || msg->data == nullptr) {
        Package::ReleaseMessage(msg);
        return;
    }

    if (pid <= 0 || pid != handle_->GetPlayerID()) {
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

void PlayerContext::PushMessage(Message *msg) {
    if (msg == nullptr || msg->data == nullptr || IsChannelClosed()) {
        Package::ReleaseMessage(msg);
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
