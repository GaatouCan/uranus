#include "Connection.h"
#include "PackagePool.h"
#include "Message.h"
#include "base/Utils.h"
#include "PackageCodec.h"
#include "../GameWorld.h"
#include "../login/LoginAuth.h"
#include "../player/PlayerManager.h"
#include "../player/PlayerContext.h"
// #include "../service/ServiceManager.h"
// #include "../service/ServiceContext.h"

#include <asio/experimental/awaitable_operators.hpp>
#include <spdlog/spdlog.h>


using namespace asio::experimental::awaitable_operators;
using uranus::network::PackageCodec;

Connection::Connection(SslStream &&stream, Gateway *gateway)
    : stream_(std::move(stream)),
      gateway_(gateway),
      output_(stream_.get_executor(), 1024),
      pid_(kInvalidPlayerID),
      watchdog_(stream_.get_executor()),
      expiration_(std::chrono::seconds(30)) {

    codec_ = make_unique<PackageCodec>(stream_);

    pool_ = make_shared<PackagePool>();
    pool_->Initial(64);

    key_ = fmt::format("{}-{}", RemoteAddress().to_string(), utils::UnixTime());
}

Connection::~Connection() {
    this->Disconnect();
}

std::string Connection::GetKey() const {
    return key_;
}

Gateway *Connection::GetGateway() const {
    return gateway_;
}

GameServer *Connection::GetGameServer() const {
    return gateway_->GetGameServer();
}

asio::ip::address Connection::RemoteAddress() const {
    return stream_.next_layer().remote_endpoint().address();
}

void Connection::ConnectToClient() {
    received_ = std::chrono::steady_clock::now();

    co_spawn(stream_.get_executor(), [self = shared_from_this()]() mutable -> awaitable<void> {
        const auto ec = co_await self->codec_->Initial();
        if (ec) {
            SPDLOG_ERROR("Connection[{}] - Failed to initial codec, error code: {}", self->key_, ec.message());
            self->Disconnect();
        }

        co_await (
            self->ReadPackage() &&
            self->WritePackage() &&
            self->Watchdog()
        );
    }, detached);
}

void Connection::Disconnect() {
    if (!IsConnected()) {
        return;
    }

    gateway_->RemoveConnection(key_, pid_);

    stream_.next_layer().close();
    output_.close();
    watchdog_.cancel();
}

bool Connection::IsConnected() const {
    return stream_.next_layer().is_open();
}

void Connection::SetPlayerID(const int64_t pid) {
    pid_ = pid;
    gateway_->OnPlayerLogin(shared_from_this());
}

int64_t Connection::GetPlayerID() const {
    if (pid_ < 0)
        return kInvalidPlayerID;
    return pid_;
}

void Connection::SendToClient(Message *msg) {
    if (msg == nullptr) return;

    if (!IsConnected()) {
        Package::ReleaseMessage(msg);
        return;
    }

    if (!output_.try_send_via_dispatch(error_code{}, msg)) {
        co_spawn(stream_.get_executor(), [self = shared_from_this(), msg]() mutable -> awaitable<void> {
            const auto [ec] = co_await self->output_.async_send(error_code{}, msg);
            if (ec == asio::experimental::error::channel_closed) {
                Package::ReleaseMessage(msg);
            }
        }, detached);
    }
}

awaitable<void> Connection::ReadPackage() {
    try {
        while (IsConnected()) {
            auto *msg = new Message();
            auto *pkg = pool_->Acquire();

            msg->type |= (Message::kFromClient | Message::kToPlayer);
            msg->session = 0;
            msg->data = reinterpret_cast<void *>(pkg);
            msg->length = sizeof(Package);

            const auto ec = co_await codec_->Decode(msg);
            if (ec) {
                SPDLOG_ERROR("Connection[{}] - Failed to read package, error: {}", key_, ec.message());
                Package::ReleaseMessage(msg);
                this->Disconnect();
                break;
            }

            if (pid_ < 0) {
                if (auto *auth = GetGameServer()->GetModule<LoginAuth>()) {
                    auth->OnPlayerLogin(shared_from_this(), static_cast<Package *>(msg->data));
                }
                Package::ReleaseMessage(msg);
                continue;
            }

            received_ = std::chrono::steady_clock::now();

            if (const auto *mgr = GetGameServer()->GetModule<PlayerManager>()) {
                if (const auto plr = mgr->FindPlayer(pid_)) {
                    plr->PushMessage(msg);
                    continue;
                }
            }

            Package::ReleaseMessage(msg);
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Connection[{}] - Exception: {}", key_, e.what());
    }
}

awaitable<void> Connection::WritePackage() {
    try {
        while (IsConnected() && output_.is_open()) {
            const auto [ec, msg] = co_await output_.async_receive();

            if (msg == nullptr || msg->data == nullptr) {
                Package::ReleaseMessage(msg);
                continue;
            }

            if (ec) {
                // TODO
                Package::ReleaseMessage(msg);
                this->Disconnect();
                break;
            }

            const auto codec_ec = co_await codec_->Encode(msg);

            Package::ReleaseMessage(msg);

            if (codec_ec) {
                this->Disconnect();
                break;
            }
        }

        while (true) {
            const bool ok = output_.try_receive([](error_code ec, Message *msg) {
                Package::ReleaseMessage(msg);
            });

            if (!ok)
                break;
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Connection[{}] - Exception: {}", key_, e.what());
    }
}

awaitable<void> Connection::Watchdog() {
    if (expiration_ <= SteadyDuration::zero())
        co_return;

    try {
        SteadyTimePoint now;

        do {
            watchdog_.expires_at(received_ + expiration_);

            if (auto [ec] = co_await watchdog_.async_wait(); ec) {
                if (ec == asio::error::operation_aborted) {
                    co_return;
                }
                break;
            }

            now = std::chrono::steady_clock::now();
        } while (received_ + expiration_ > now);

        if (IsConnected()) {
            this->Disconnect();
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Connection[{}] - Exception: {}", key_, e.what());
    }
}
