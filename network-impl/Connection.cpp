#include "Connection.h"

#include <asio/experimental/awaitable_operators.hpp>

using namespace asio::experimental::awaitable_operators;

namespace uranus::network {
    Connection::Connection(SslStream &&stream)
        : stream_(std::move(stream)),
          server_(nullptr) {
    }

    Connection::~Connection() {
    }

    void Connection::ConnectToClient() {
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
        if (!stream_.next_layer().is_open()) {
            return;
        }

        stream_.next_layer().close();

        // TODO
    }

    awaitable<void> Connection::ReadPackage() {
        try {

        } catch (const std::exception &e) {
            // TODO
        }
    }

    awaitable<void> Connection::WritePackage() {
        try {

        } catch (const std::exception &e) {
            // TODO
        }
    }

    awaitable<void> Connection::Watchdog() {
    }
}
