#include "Connection.h"
#include "Gateway.h"
#include "PackagePool.h"
#include "Message.h"
#include "base/Utils.h"
#include "PackageCodec.h"
#include "ConfigModule.h"
#include "../GameWorld.h"
#include "../login/LoginAuth.h"
#include "../player/PlayerManager.h"
#include "../player/PlayerContext.h"
#include "../other/FixedPackageID.h"

#include <asio/experimental/awaitable_operators.hpp>
#include <spdlog/spdlog.h>


using namespace asio::experimental::awaitable_operators;
using uranus::network::PackageCodec;
using uranus::config::ConfigModule;


Connection::Connection(SslStream &&stream, Gateway *gateway)
    : stream_(std::move(stream)),
      gateway_(gateway),
      pid_(kInvalidPlayerID),
      watchdog_(stream_.get_executor()),
      expiration_(std::chrono::seconds(30)) {

    const auto &cfg = GetGameServer()->GetModule<ConfigModule>()->GetServerConfig();

    // Read the parameters from config
    const auto expiration               = cfg["server"]["network"]["expiration"].as<int>();
    const auto output_size              = cfg["server"]["network"]["outputBuffer"].as<int>();
    const auto initial_capacity         = cfg["server"]["network"]["recycler"]["initialCapacity"].as<int>();
    const auto minimum_capacity         = cfg["server"]["network"]["recycler"]["minimumCapacity"].as<int>();
    const auto half_collect             = cfg["server"]["network"]["recycler"]["halfCollect"].as<int>();
    const auto full_collect             = cfg["server"]["network"]["recycler"]["fullCollect"].as<int>();
    const auto collect_threshold    = cfg["server"]["network"]["recycler"]["collectThreshold"].as<double>();
    const auto collect_rate         = cfg["server"]["network"]["recycler"]["collectRate"].as<double>();

    // Set the watchdog timeout
    expiration_ = std::chrono::seconds(expiration);

    // Create the package codec
    codec_ = make_unique<PackageCodec>(stream_);

    // Create the package pool
    pool_ = make_shared<PackagePool>();

    // Set up the parameters of package pool
    pool_->SetHalfCollect(half_collect);
    pool_->SetFullCollect(full_collect);
    pool_->SetMinimumCapacity(minimum_capacity);
    pool_->SetCollectThreshold(collect_threshold);
    pool_->SetCollectRate(collect_rate);

    // Initial the package pool
    pool_->Initial(initial_capacity);

    // Create the output channel
    output_ = make_unique<ConcurrentChannel<Message>>(stream_.get_executor(), output_size);

    // Generate the unique key
    key_ = fmt::format("{}-{}", RemoteAddress().to_string(), utils::UnixTime());
}

Connection::~Connection() {
    Disconnect();
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

Message Connection::BuildMessage() const {
    Message msg;

    auto *pkg = pool_->Acquire();

    msg.data = reinterpret_cast<void *>(pkg);
    msg.length = sizeof(Package);

    return msg;
}

asio::ip::address Connection::RemoteAddress() const {
    return stream_.next_layer().remote_endpoint().address();
}

void Connection::SetExpiration(const int sec) {
    expiration_ = std::chrono::seconds(sec);
}

void Connection::ConnectToClient() {
    received_ = std::chrono::steady_clock::now();

    co_spawn(stream_.get_executor(), [self = shared_from_this()]() mutable -> awaitable<void> {
        const auto ec = co_await self->codec_->Initial();
        if (ec) {
            SPDLOG_ERROR("Connection[{}] - Failed to initial codec, error code: {}", self->key_, ec.message());
            self->Disconnect();
        }

        SPDLOG_INFO("Connection[{}] - Initial codec success.", self->key_);

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

    // Close the tcp socket
    stream_.next_layer().close();

    // Close the output channel
    output_->cancel();
    output_->close();

    // cancel the watchdog timer
    watchdog_.cancel();

    // Remove the connection from Gateway
    if (!key_.empty()) {
        gateway_->RemoveConnection(key_, pid_);
    }

    // Tell PlayerManager the client is offline(If player id is valid)
    if (pid_ > 0) {
        if (auto *mgr = GetGameServer()->GetModule<PlayerManager>()) {
            mgr->OnPlayerLogout(pid_);
        }
    }
}

bool Connection::IsConnected() const {
    return stream_.next_layer().is_open();
}

void Connection::SetPlayerID(const int64_t pid) {
    pid_ = pid;
    SPDLOG_INFO("Connection[{}] - Bind to player id: {}", key_, pid);
}

int64_t Connection::GetPlayerID() const {
    if (pid_ < 0)
        return kInvalidPlayerID;
    return pid_;
}

void Connection::SendToClient(const Message &msg) {
    if (!IsConnected()) {
        Package::ReleaseMessage(msg);
        return;
    }

    if (!output_->try_send_via_dispatch(error_code{}, msg)) {
        co_spawn(stream_.get_executor(), [self = shared_from_this(), msg]() mutable -> awaitable<void> {
            const auto [ec] = co_await self->output_->async_send(error_code{}, msg);
            if (ec == asio::experimental::error::channel_closed ||
                ec == asio::error::operation_aborted) {
                Package::ReleaseMessage(msg);
            }
        }, detached);
    }
}

awaitable<void> Connection::ReadPackage() {
    try {
        while (IsConnected()) {
            Message msg;
            auto *pkg = pool_->Acquire();

            msg.type        = (Message::kFromClient | Message::kToPlayer);
            msg.session     = 0;
            msg.source      = pid_;
            msg.data        = reinterpret_cast<void *>(pkg);
            msg.length      = sizeof(Package);

            const auto ec = co_await codec_->Decode(msg);
            if (ec) {
                SPDLOG_ERROR("Connection[{}] - Failed to read package, error: {}", key_, ec.message());
                Package::ReleaseMessage(msg);
                Disconnect();
                break;
            }

            if (pid_ < 0) {
                if (auto *auth = GetGameServer()->GetModule<LoginAuth>()) {
                    auth->OnPlayerLogin(shared_from_this(), static_cast<Package *>(msg.data));
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
        while (IsConnected() && output_->is_open()) {
            const auto [ec, msg] = co_await output_->async_receive();

            if (ec == asio::error::operation_aborted ||
                ec == asio::experimental::error::channel_closed) {
                Package::ReleaseMessage(msg);
                break;
            }

            if (msg.data == nullptr) {
                Package::ReleaseMessage(msg);
                continue;
            }

            if (ec) {
                SPDLOG_ERROR("Connection[{}] - Failed to get message, error: {}", key_, ec.message());
                Package::ReleaseMessage(msg);
                continue;
            }

            // If failed to write, close the connection
            if (const auto codec_ec = co_await codec_->Encode(msg)) {
                SPDLOG_ERROR("Connection[{}] - Failed to write message, error: {}", key_, codec_ec.message());
                Package::ReleaseMessage(msg);
                Disconnect();
                break;
            }

            auto *pkg = static_cast<Package *>(msg.data);

            if (pkg->GetPackageID() == static_cast<int>(FixedPackageID::kLoginRepeated) ||
                pkg->GetPackageID() == static_cast<int>(FixedPackageID::kLoginFailed)) {
                // Clean the key and player id to sure that will not affect the new connection
                key_.clear();
                pid_ = kInvalidPlayerID;
                Package::ReleaseMessage(msg);
                Disconnect();
                break;
            }

            Package::ReleaseMessage(msg);
        }

        while (output_->try_receive([](auto, const auto &msg) {
            Package::ReleaseMessage(msg);
        })) {}
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
            Disconnect();
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Connection[{}] - Exception: {}", key_, e.what());
    }
}
