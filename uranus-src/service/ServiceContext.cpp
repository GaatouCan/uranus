#include "ServiceContext.h"
#include "AbstractService.h"
#include "Message.h"
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

    channel_ = make_unique<ConcurrentChannel<Message>>(GetIOContext(), channelSize);

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

AbstractService *ServiceContext::GetService() const {
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

Message ServiceContext::BuildMessage() {
    Message msg;

    msg.type = Message::kFromService;
    msg.source = handle_->GetServiceID();
    msg.session = 0;

    auto *pkg = pool_->Acquire();

    msg.data = pkg;
    msg.length = sizeof(Package);

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

void ServiceContext::Send(const int64_t target, const Message &msg) {
    if (!handle_.IsValid()) {
        throw std::runtime_error(std::format(
            "{} - ServiceContext[{:p}] - Service Handle is invalid",
            __FUNCTION__, static_cast<const void *>(this)));
    }

    if (msg.data == nullptr) {
        this->DisposeMessage(msg);
        return;
    }

    if (msg.type & Message::kToServer) {
        // TODO
    } else if (msg.type & Message::kToService) {
        if (target < 0 || target == handle_->GetServiceID()) {
            this->DisposeMessage(msg);
            return;
        }
        if (const auto *mgr = GetGameServer()->GetModule<ServiceManager>()) {
            if (const auto ser = mgr->FindService(target)) {
                ser->PushMessage(msg);
                return;
            }
        }
    } else if (msg.type & Message::kToPlayer) {
        if (target <= 0) {
            this->DisposeMessage(msg);
            return;
        }
        if (const auto *mgr = GetGameServer()->GetModule<PlayerManager>()) {
            if (const auto plr = mgr->FindPlayer(target)) {
                plr->PushMessage(msg);
                return;
            }
        }
    } else if (msg.type & Message::kToClient) {
        if (target <= 0) {
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

void ServiceContext::CleanUp() {
    ActorContext::CleanUp();
    if (handle_.IsValid()) {
        handle_->Stop();
    }
    handle_.Release();
}

void ServiceContext::HandleMessage(const Message &msg) {
    if (msg.data == nullptr)
        return;

    if ((msg.type & Message::kToService) == 0) {
        this->DisposeMessage(msg);
        return;
    }

    if (msg.type & Message::kRequest) {
        if ((msg.type & Message::kFromService) == 0 &&
            (msg.type & Message::kFromServer) == 0 &&
            (msg.type & Message::kFromPlayer) == 0) {
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
        } else if (msg.type & Message::kFromPlayer) {
            res.type |= Message::kToPlayer;
            if (const auto *mgr = GetGameServer()->GetModule<PlayerManager>()) {
                if (const auto plr = mgr->FindPlayer(msg.source)) {
                    plr->PushMessage(res);
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

void ServiceContext::DisposeMessage(const Message &msg) {
    Package::ReleaseMessage(msg);
}

void ServiceContext::SendToService(const std::string &name, const Message &msg) {
    if (!handle_.IsValid()) {
        throw std::runtime_error(std::format(
            "{} - ServiceContext[{:p}] - Service Handle is invalid",
            __FUNCTION__, static_cast<const void *>(this)));
    }

    if (msg.data == nullptr)
        return;

    if (name.empty()) {
        this->DisposeMessage(msg);
        return;
    }

    int64_t target = kInvalidServiceID;

    if (const auto *mgr = GetGameServer()->GetModule<ServiceManager>()) {
        target = mgr->FindServiceID(name);
    }

    if (target >= 0 || target != handle_->GetServiceID()) {
        this->Send(target, msg);
        return;
    }

    this->DisposeMessage(msg);
}

void ServiceContext::RemoteCall(const int64_t target, Message req, SessionNode &&node) {
    auto dispose = [](SessionNode &&sess) {
        auto alloc = asio::get_associated_allocator(
            sess.handler,
            asio::recycling_allocator<void>()
        );
        asio::dispatch(
            sess.work.get_executor(),
            asio::bind_allocator(
                alloc,
                [handler = std::move(sess.handler)]() mutable {
                    std::move(handler)(std::nullopt);
                }
            )
        );
    };

    if (req.data == nullptr || ((req.type  & Message::kRequest) == 0)) {
        dispose(std::move(node));
        this->DisposeMessage(req);
        return;
    }

    if (req.type & Message::kToService) {
        if (target == handle_->GetServiceID()) {
            dispose(std::move(node));
            this->DisposeMessage(req);
            return;
        }

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
    } else if (req.type & Message::kToPlayer) {
        if (const auto *mgr = GetGameServer()->GetModule<PlayerManager>()) {
            if (const auto plr = mgr->FindPlayer(target)) {
                const auto sess_id = this->AllocateSessionID();

                req.type |= Message::kRequest;
                req.session = sess_id;

                plr->PushMessage(req);

                this->PushSession(sess_id, std::move(node));
                return;
            }
        }
    }

    dispose(std::move(node));
    this->DisposeMessage(req);
}

void ServiceContext::SetUpService(ServiceHandle &&handle) {
    handle_ = std::move(handle);
    SetUpActor();
}
