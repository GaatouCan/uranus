#include "BaseConnection.h"

#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/experimental/awaitable_operators.hpp>

namespace uranus::network {

    using namespace asio::experimental::awaitable_operators;
    using asio::detached;
    using asio::co_spawn;

    BaseConnection::BaseConnection(TcpSocket &&socket)
        : socket_(std::move(socket)),
          watchdog_(socket_.get_executor()),
          expiration_(-1) {

        const auto now = std::chrono::system_clock::now();
        const auto durationSinceEpoch = now.time_since_epoch();
        const auto secondsSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(durationSinceEpoch);

#ifdef URANUS_SSL
        const auto key = std::format("{}-{}", socket_.next_layer().remote_endpoint().address().to_string(), secondsSinceEpoch.count());
#else
        const auto key = std::format("{}-{}", socket_.remote_endpoint().address().to_string(), secondsSinceEpoch.count());
#endif

        attr_.set("CONNECTION_KEY", key);
    }

    BaseConnection::~BaseConnection() {
    }

    TcpSocket &BaseConnection::socket() {
        return socket_;
    }

    void BaseConnection::connect() {
        // Mark the received timestamp
        received_ = std::chrono::steady_clock::now();

        co_spawn(socket_.get_executor(), [self = shared_from_this()]() -> awaitable<void> {
#ifdef URANUS_SSL
            if (const auto [ec] = co_await self->socket_.async_handshake(asio::ssl::stream_base::server); ec) {
                self->onErrorCode(ec);
                self->disconnect();
                co_return;
            }
#endif

            self->onConnect();

            co_await (
                self->readLoop() &&
                self->writeLoop() &&
                self->watchdog()
            );
        }, detached);
    }

    void BaseConnection::disconnect() {
        if (!isConnected())
            return;

        asio::dispatch(socket_.get_executor(), [self = shared_from_this()]() {
            if (!self->isConnected())
                return;

#ifdef URANUS_SSL
            self->socket_.next_layer().close();
#else
            self->socket_.close();
#endif
            self->watchdog_.cancel();

            // Call the virtual method
            self->onDisconnect();
        });
    }

    bool BaseConnection::isConnected() const {
#ifdef URANUS_SSL
        return socket_.next_layer().is_open();
#else
        return socket_.is_open();
#endif
    }

    asio::ip::address BaseConnection::remoteAddress() const {
#ifdef URANUS_SSL
        return socket_.next_layer().remote_endpoint().address();
#else
        return socket_.remote_endpoint().address();
#endif
    }

    void BaseConnection::setExpirationSecond(const int sec) {
        expiration_ = std::chrono::seconds(sec);
    }

    AttributeMap &BaseConnection::attr() {
        return attr_;
    }

    const AttributeMap &BaseConnection::attr() const {
        return attr_;
    }

    awaitable<void> BaseConnection::watchdog() {
        if (expiration_ <= SteadyDuration::zero())
            co_return;

        try {
            do {
                watchdog_.expires_at(received_ + expiration_);

                if (auto [ec] = co_await watchdog_.async_wait(); ec) {
                    if (ec != asio::error::operation_aborted) {
                        // Not excepted error
                        this->onErrorCode(ec);
                    }

                    if (isConnected()) {
                        this->disconnect();
                    }

                    co_return;
                }

                this->onTimeout();

                if (isConnected()) {
                    this->disconnect();
                }
            } while (received_ + expiration_ > std::chrono::steady_clock::now());
        } catch (std::exception &e) {
            this->onException(e);
            this->disconnect();
        }
    }
}
