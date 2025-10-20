#include "Connection.h"
#include "PackagePool.h"
#include "Message.h"
#include "base/Utils.h"
#include "PackageCodec.h"
#include "../GameWorld.h"

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

    key_ = fmt::format("{}-{}", stream_.next_layer().remote_endpoint().address().to_string(), utils::UnixTime());
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

void Connection::ConnectToClient() {
    received_ = std::chrono::steady_clock::now();

    co_spawn(stream_.get_executor(), [self = shared_from_this()]() mutable -> awaitable<void> {
        const auto [ec] = co_await self->stream_.async_handshake(asio::ssl::stream_base::server);
        if (ec) {
            // TODO
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

    gateway_->RemoveConnection(key_);

    stream_.next_layer().close();
    output_.close();
    watchdog_.cancel();
}

bool Connection::IsConnected() const {
    return stream_.next_layer().is_open();
}

void Connection::SendToClient(Message *msg) {
    if (msg == nullptr) return;

    if (!IsConnected()) {
        auto *pkg = static_cast<Package *>(msg->data);
        pkg->Recycle();
        delete msg;
        return;
    }

    output_.try_send_via_dispatch(error_code{}, msg);
}

awaitable<void> Connection::ReadPackage() {
    try {
        while (IsConnected()) {
            auto *msg = new Message();
            auto *pkg = pool_->Acquire();

            msg->type |= Message::kFromClient;
            msg->session = 0;
            msg->data = reinterpret_cast<void *>(pkg);
            msg->length = sizeof(Package);

            const auto ec = co_await codec_->Decode(msg);
            if (ec) {
                // TODO:

                pkg->Recycle();
                delete msg;
                this->Disconnect();
            }

            // TODO: Handle Message
        }
    } catch (const std::exception &e) {
        // TODO
    }
}

awaitable<void> Connection::WritePackage() {
    try {
        while (IsConnected() && output_.is_open()) {
            const auto [ec, msg] = co_await output_.async_receive();
            if (ec) {
                // TODO
                break;
            }

            if (msg == nullptr)
                continue;

            const auto codec_ec = co_await codec_->Encode(msg);

            auto *pkg = static_cast<Package *>(msg->data);
            pkg->Recycle();

            delete msg;

            if (codec_ec) {
                this->Disconnect();
                break;
            }
        }

        while (true) {
            const bool ok = output_.try_receive([](error_code ec, Message *msg) {
                if (msg == nullptr)
                    return;

                if (msg->data != nullptr) {
                    auto *pkg = static_cast<Package *>(msg->data);
                    pkg->Recycle();
                }

                delete msg;
            });

            if (!ok)
                break;
        }
    } catch (const std::exception &e) {
        // TODO
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
        // SPDLOG_ERROR("{} - Connection[{}], Exception: {}", __FUNCTION__, key_, e.what());
    }
}
