#include "PlayerContext.h"
#include "AbstractPlayer.h"
#include "Message.h"
#include "Package.h"
#include "PackagePool.h"
#include "ConfigModule.h"
#include "../GameWorld.h"
#include "../gateway/Connection.h"
#include "../gateway/Gateway.h"
#include "../service/ServiceContext.h"
#include "../service/ServiceManager.h"


using uranus::network::Package;
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
    channel_ = make_unique<ConcurrentChannel<Message>>(GetIOContext(), channel_size);

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

AbstractPlayer *PlayerContext::GetPlayer() const {
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

Message PlayerContext::BuildMessage() {
    Message msg;

    msg.type = Message::kFromPlayer;
    msg.source = handle_->GetPlayerID();
    msg.session = 0;

    auto *pkg = pool_->Acquire();

    msg.data = pkg;
    msg.length = sizeof(Package);

    return msg;
}

void PlayerContext::Send(const int64_t target, const Message &msg) {
    if (!handle_.IsValid()) {
        throw std::runtime_error(std::format(
            "{} - PlayerContext[{:p}] - Player Handle is invalid",
            __FUNCTION__, static_cast<const void *>(this)));
    }

    if (msg.data == nullptr) {
        return;
    }

    if (msg.type & Message::kToServer) {
        // TODO
    } else if (msg.type & Message::kToService) {
        if (target < 0) {
            this->DisposeMessage(msg);
            return;
        }
        if (const auto *mgr = GetGameServer()->GetModule<ServiceManager>()) {
            if (const auto ser = mgr->FindService(target)) {
                ser->PushMessage(msg);
                return;
            }
        }
    } else if (msg.type & Message::kToClient) {
        if (target != handle_->GetPlayerID()) {
            this->DisposeMessage(msg);
            return;
        }
        if (const auto *gateway = GetGameServer()->GetModule<Gateway>()) {
            if (const auto conn = gateway->FindConnection(target)) {
                conn->SendToClient(msg);
                return;
            }
        }
    }

    this->DisposeMessage(msg);
}

void PlayerContext::SendToService(const std::string &name, const Message &msg) {
    if (!handle_.IsValid()) {
        throw std::runtime_error(std::format(
            "{} - PlayerContext[{:p}] - Player Handle is invalid",
            __FUNCTION__, static_cast<const void *>(this)));
    }

    if (msg.data == nullptr)
        return;

    if (name.empty()) {
        this->DisposeMessage(msg);
        return;
    }

    if (const auto *mgr = GetGameServer()->GetModule<ServiceManager>()) {
        if (const auto ser = mgr->FindService(name)) {
            ser->PushMessage(msg);
            return;
        }
    }

    this->DisposeMessage(msg);
}

void PlayerContext::RemoteCall(const int64_t target, Message req, SessionNode &&node) {
    if (req.data == nullptr || ((req.type & Message::kRequest) == 0)) {
        auto alloc = asio::get_associated_allocator(node.handler, asio::recycling_allocator<void>());
        asio::dispatch(node.work.get_executor(), asio::bind_allocator(alloc, [handler = std::move(node.handler)]() mutable {
            std::move(handler)(std::nullopt);
        }));

        this->DisposeMessage(req);
        return;
    }

    if (req.type & Message::kToService) {
        if (const auto *mgr = GetGameServer()->GetModule<ServiceManager>()) {
            if (const auto ser = mgr->FindService(target)) {
                const auto sess_id = this->AllocateSessionID();

                req.type |= Message::kRequest;
                req.session = sess_id;

                ser->PushMessage(req);

                this->PushSession(sess_id, std::move(node));
                return;
            }
        }
    }

    auto alloc = asio::get_associated_allocator(node.handler, asio::recycling_allocator<void>());
    asio::dispatch(node.work.get_executor(), asio::bind_allocator(alloc, [handler = std::move(node.handler)]() mutable {
        std::move(handler)(std::nullopt);
    }));

    this->DisposeMessage(req);
}

void PlayerContext::CleanUp() {
    ActorContext::CleanUp();
    if (handle_) {
        handle_->Stop();
    }
    handle_.Release();
}

void PlayerContext::HandleMessage(const Message &msg) {
    if (msg.data == nullptr)
        return;

    if ((msg.type & Message::kToPlayer)== 0) {
        this->DisposeMessage(msg);
        return;
    }

    if (msg.type & Message::kRequest) {
        // Only Handle From Service Or From Server
        if ((msg.type & Message::kFromService) == 0 &&
            (msg.type & Message::kFromServer) == 0) {
            this->DisposeMessage(msg);
            return;
        }

        auto res = this->BuildMessage();

        res.type |= Message::kResponse;
        res.session = msg.session;

        handle_->OnRequest(msg, res);

        if (msg.type & Message::kFromService) {
            res.type |= Message::kToService;
            if (const auto *mgr = GetGameServer()->GetModule<ServiceManager>()) {
                if (const auto ser = mgr->FindService(msg.source)) {
                    ser->PushMessage(res);
                    return;
                }
            }
        } else if (msg.type & Message::kFromServer) {
            // TODO
        }

        this->DisposeMessage(res);
    } else if (msg.type & Message::kResponse) {
        if (msg.session < 0) {
            this->DisposeMessage(msg);
            return;
        }

        const auto op = this->TakeSession(msg.session);
        if (!op.has_value()) {
            this->DisposeMessage(msg);
            return;
        }

        auto [handler, work] = std::move(op.value());
        auto alloc = asio::get_associated_allocator(handler, asio::recycling_allocator<void>());
        asio::dispatch(
            work.get_executor(),
            asio::bind_allocator(
                alloc,
                [handle = std::move(handler), msg]() mutable {
                    std::move(handle)(msg);
                }
            )
        );
    } else {
        handle_->OnReceive(msg);
    }

    this->DisposeMessage(msg);
}

void PlayerContext::DisposeMessage(const Message &msg) {
    Package::ReleaseMessage(msg);
}

void PlayerContext::SetUpPlayer(PlayerHandle &&handle) {
    handle_ = std::move(handle);
    this->SetUpActor();
}
