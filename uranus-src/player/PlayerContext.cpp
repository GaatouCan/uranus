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
using uranus::ChannelNode;


PlayerContext::PlayerContext(GameWorld *world)
    : ActorContext(world) {

    const auto &cfg = GetGameServer()->GetModule<ConfigModule>()->GetServerConfig();

    const auto channel_size             = cfg["player"]["queueBuffer"].as<int>();
    const auto minimum_capacity         = cfg["player"]["recycler"]["minimumCapacity"].as<int>();
    const auto half_collect             = cfg["player"]["recycler"]["halfCollect"].as<int>();
    const auto full_collect             = cfg["player"]["recycler"]["fullCollect"].as<int>();
    const auto collect_threshold    = cfg["player"]["recycler"]["collectThreshold"].as<double>();
    const auto collect_rate         = cfg["player"]["recycler"]["collectRate"].as<double>();

    // Create the processing channel
    channel_ = make_unique<ConcurrentChannel<unique_ptr<ChannelNode>>>(GetIOContext(), channel_size);

    // Create the package pool
    pool_ = make_shared<PackagePool>();

    // Set up the parameters of package pool
    pool_->SetHalfCollect(half_collect);
    pool_->SetFullCollect(full_collect);
    pool_->SetMinimumCapacity(minimum_capacity);
    pool_->SetCollectThreshold(collect_threshold);
    pool_->SetCollectRate(collect_rate);
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

    const auto &cfg = GetGameServer()->GetModule<ConfigModule>()->GetServerConfig();
    const auto initialCapacity  = cfg["player"]["recycler"]["initialCapacity"].as<int>();

    pool_->Initial(initialCapacity);

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
    msg->source = handle_->GetPlayerID();
    msg->session = 0;

    auto *pkg = pool_->Acquire();

    msg->data = pkg;
    msg->length = sizeof(Package);

    return msg;
}

void PlayerContext::Send(const int64_t target, Message *msg) {
    if (!handle_.IsValid()) {
        throw std::runtime_error(std::format(
            "{} - PlayerContext[{:p}] - Player Handle is invalid",
            __FUNCTION__, static_cast<const void *>(this)));
    }

    if (msg == nullptr || msg->data == nullptr) {
        Package::ReleaseMessage(msg);
        return;
    }

    if (msg->type & Message::kToServer) {
        // TODO
    } else if (msg->type & Message::kToService) {
        if (target < 0) {
            Package::ReleaseMessage(msg);
            return;
        }
        if (const auto *mgr = GetGameServer()->GetModule<ServiceManager>()) {
            if (const auto ser = mgr->FindService(target)) {
                ser->PushMessage(msg);
                return;
            }
        }
    } else if (msg->type & Message::kToClient) {
        if (target != handle_->GetPlayerID()) {
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

void PlayerContext::RemoteCall(const int64_t target, Message *msg, std::unique_ptr<SessionNode> &&node) {
    if (msg == nullptr || msg->data == nullptr || ((msg->type  & Message::kRequest) == 0)) {
        auto alloc = asio::get_associated_allocator(node->handler, asio::recycling_allocator<void>());
        asio::dispatch(node->work.get_executor(), asio::bind_allocator(alloc, [handler = std::move(node->handler)]() mutable {
            std::move(handler)(nullptr);
        }));

        Package::ReleaseMessage(msg);
        return;
    }

    if (msg->type & Message::kToService) {
        if (const auto *mgr = GetGameServer()->GetModule<ServiceManager>()) {
            if (const auto ser = mgr->FindService(target)) {
                const auto sess_id = this->AllocateSessionID();

                msg->type |= Message::kRequest;
                msg->session = sess_id;

                ser->PushMessage(msg);

                this->PushSession(sess_id, std::move(node));
                return;
            }
        }
    }

    auto alloc = asio::get_associated_allocator(node->handler, asio::recycling_allocator<void>());
    asio::dispatch(node->work.get_executor(), asio::bind_allocator(alloc, [handler = std::move(node->handler)]() mutable {
        std::move(handler)(nullptr);
    }));

    Package::ReleaseMessage(msg);
}

void PlayerContext::PushMessage(Message *msg) {
    if (msg == nullptr || msg->data == nullptr || IsChannelClosed()) {
        Package::ReleaseMessage(msg);
        return;
    }

    auto node = make_unique<PackageNode>();
    node->SetMessage(msg);

    this->PushNode(std::move(node));
}

void PlayerContext::OnRequest(Message *req) {
    // msg will be released in ::~PackageNode
    if (req == nullptr || req->data == nullptr || (req->type & Message::kRequest) == 0) {
        return;
    }

    // Only Handle From Service Or From Server
    if ((req->type & Message::kFromService) == 0 &&
        (req->type & Message::kFromServer) == 0) {
        return;
    }

    auto *res = this->BuildMessage();

    res->type |= Message::kResponse;
    res->session = req->session;

    handle_->OnRequest(req, res);

    if (req->type & Message::kFromService) {
        res->type |= Message::kToService;
        if (const auto *mgr = GetGameServer()->GetModule<ServiceManager>()) {
            if (const auto ser = mgr->FindService(req->source)) {
                ser->PushMessage(res);
                return;
            }
        }
    } else if (req->type & Message::kFromServer) {
        // TODO
    }

    // If not return earlier, release the response
    Package::ReleaseMessage(res);
}

void PlayerContext::OnResponse(Message *res) {
    if (res == nullptr || res->data == nullptr || (res->type & Message::kResponse) == 0 || res->session < 0) {
        Package::ReleaseMessage(res);
        return;
    }

    const auto sess = this->TakeSession(res->session);
    if (sess == nullptr) {
        Package::ReleaseMessage(res);
        return;
    }

    auto alloc = asio::get_associated_allocator(sess->handler, asio::recycling_allocator<void>());
    asio::dispatch(
        sess->work.get_executor(),
        asio::bind_allocator(
            alloc,
            [handler = std::move(sess->handler), res]() mutable {
                std::move(handler)(res);
            }
        )
    );
}

void PlayerContext::CleanUp() {
    ActorContext::CleanUp();
    if (handle_) {
        handle_->Stop();
    }
    handle_.Release();
}

void PlayerContext::SetUpPlayer(PlayerHandle &&handle) {
    handle_ = std::move(handle);
    this->SetUpActor();
}
