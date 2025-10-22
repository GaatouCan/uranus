#include "ServiceContext.h"
#include "AbstractService.h"
#include "Message.h"
#include "PackageNode.h"
#include "PackagePool.h"
#include "ServiceManager.h"
#include "ConfigModule.h"
#include "../GameWorld.h"
#include "../gateway/Gateway.h"
#include "../gateway/Connection.h"
#include "../player/PlayerContext.h"
#include "../player/PlayerManager.h"


using uranus::ChannelNode;
using uranus::network::Package;
using uranus::network::PackageNode;
using uranus::config::ConfigModule;


ServiceContext::ServiceContext(GameWorld *world)
    : ActorContext(world) {
    const auto &cfg = GetGameServer()->GetModule<ConfigModule>()->GetServerConfig();

    const auto channelSize = cfg["service"]["queueBuffer"].as<int>();
    const auto minimumCapacity = cfg["service"]["recycler"]["minimumCapacity"].as<int>();
    const auto halfCollect = cfg["service"]["recycler"]["halfCollect"].as<int>();
    const auto fullCollect = cfg["service"]["recycler"]["fullCollect"].as<int>();
    const auto collectThreshold = cfg["service"]["recycler"]["collectThreshold"].as<double>();
    const auto collectRate = cfg["service"]["recycler"]["collectRate"].as<double>();

    channel_ = make_unique<ConcurrentChannel<unique_ptr<ChannelNode>>>(GetIOContext(), channelSize);

    pool_ = make_shared<PackagePool>();

    pool_->SetHalfCollect(halfCollect);
    pool_->SetFullCollect(fullCollect);
    pool_->SetMinimumCapacity(minimumCapacity);
    pool_->SetCollectThreshold(collectThreshold);
    pool_->SetCollectRate(collectRate);
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

    const auto &cfg = GetGameServer()->GetModule<ConfigModule>()->GetServerConfig();
    const auto initialCapacity  = cfg["service"]["recycler"]["initialCapacity"].as<int>();

    pool_->Initial(initialCapacity);

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

void ServiceContext::Send(int64_t target, Message *msg) {
    if (!handle_.IsValid()) {
        throw std::runtime_error(std::format(
            "{} - ServiceContext[{:p}] - Service Handle is invalid",
            __FUNCTION__, static_cast<const void *>(this)));
    }

    if (msg == nullptr || msg->data == nullptr) {
        Package::ReleaseMessage(msg);
        return;
    }

    if (msg->type & Message::kToServer) {
        // TODO
    } else if (msg->type & Message::kToService) {
        if (target < 0 || target == handle_->GetServiceID()) {
            Package::ReleaseMessage(msg);
            return;
        }
        if (const auto *mgr = GetGameServer()->GetModule<ServiceManager>()) {
            if (const auto ser = mgr->FindService(target)) {
                ser->PushMessage(msg);
                return;
            }
        }
    } else if (msg->type & Message::kToPlayer) {
        if (target <= 0) {
            Package::ReleaseMessage(msg);
            return;
        }
        if (const auto *mgr = GetGameServer()->GetModule<PlayerManager>()) {
            if (const auto plr = mgr->FindPlayer(target)) {
                plr->PushMessage(msg);
                return;
            }
        }
    } else if (msg->type & Message::kToClient) {
        if (target <= 0) {
            Package::ReleaseMessage(msg);
            return;
        }
        if (const auto *gateway = GetGameServer()->GetModule<Gateway>()) {
            if (const auto conn = gateway->FindConnection(target)) {
                conn->SendToClient(msg);
                return;
            }
        }
    }

    Package::ReleaseMessage(msg);
}

void ServiceContext::CleanUp() {
    if (handle_.IsValid()) {
        handle_->Stop();
    }
    handle_.Release();
}

// void ServiceContext::SendToService(const int64_t target, Message *msg) {
//     if (!handle_.IsValid()) {
//         throw std::runtime_error(std::format(
//             "{} - ServiceContext[{:p}] - Service Handle is invalid",
//             __FUNCTION__, static_cast<const void *>(this)));
//     }
//
//     if (msg == nullptr || msg->data == nullptr || target < 0 || target == handle_->GetServiceID()) {
//         Package::ReleaseMessage(msg);
//         return;
//     }
//
//     msg->type |= Message::kToService;
//
//     if (const auto *mgr = GetGameServer()->GetModule<ServiceManager>()) {
//         if (const auto ser = mgr->FindService(target)) {
//             ser->PushMessage(msg);
//             return;
//         }
//     }
//
//     Package::ReleaseMessage(msg);
// }

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
        msg->type |= Message::kToService;
        this->Send(target, msg);
        return;
    }

    Package::ReleaseMessage(msg);
}

// void ServiceContext::SendToPlayer(const int64_t pid, Message *msg) {
//     if (!handle_.IsValid()) {
//         throw std::runtime_error(std::format(
//             "{} - ServiceContext[{:p}] - Service Handle is invalid",
//             __FUNCTION__, static_cast<const void *>(this)));
//     }
//
//     if (msg == nullptr || msg->data == nullptr || pid <= 0) {
//         Package::ReleaseMessage(msg);
//         return;
//     }
//
//     msg->type |= Message::kToPlayer;
//
//     if (const auto *mgr = GetGameServer()->GetModule<PlayerManager>()) {
//         if (const auto plr = mgr->FindPlayer(pid)) {
//             plr->PushMessage(msg);
//             return;
//         }
//     }
//
//     Package::ReleaseMessage(msg);
// }
//
// void ServiceContext::SendToClient(const int64_t pid, Message *msg) {
//     if (!handle_.IsValid()) {
//         throw std::runtime_error(std::format(
//             "{} - ServiceContext[{:p}] - Service Handle is invalid",
//             __FUNCTION__, static_cast<const void *>(this)));
//     }
//
//     if (msg == nullptr || msg->data == nullptr || pid <= 0) {
//         Package::ReleaseMessage(msg);
//         return;
//     }
//
//     msg->type |= Message::kToClient;
//
//     if (const auto *gateway = GetGameServer()->GetModule<Gateway>()) {
//         if (const auto conn = gateway->FindConnection(pid)) {
//             conn->SendToClient(msg);
//             return;
//         }
//     }
//
//     Package::ReleaseMessage(msg);
// }

void ServiceContext::PushMessage(Message *msg) {
    if (msg == nullptr || msg->data == nullptr || IsChannelClosed()) {
        Package::ReleaseMessage(msg);
        return;
    }

    auto node = make_unique<PackageNode>();
    node->SetMessage(msg);

    this->PushNode(std::move(node));
}

void ServiceContext::SetUpService(ServiceHandle &&handle) {
    handle_ = std::move(handle);
    SetUpActor();
}
