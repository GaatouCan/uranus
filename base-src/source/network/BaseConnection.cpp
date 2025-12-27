#include "BaseConnection.h"

#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/experimental/awaitable_operators.hpp>

using namespace asio::experimental::awaitable_operators;
using asio::detached;
using asio::co_spawn;

namespace uranus::network
{
    BaseConnection::BaseConnection(TcpSocket&& socket)
        : socket_(std::move(socket)),
          watchdog_(socket_.get_executor()),
          expiration_(-1)
    {
        const auto now = std::chrono::system_clock::now();
        const auto durationSinceEpoch = now.time_since_epoch();
        const auto secondsSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(durationSinceEpoch);

#ifdef URANUS_SSL
        key_ = std::format("{}-{}", socket_.next_layer().remote_endpoint().address().to_string(),
                           secondsSinceEpoch.count());
#else
        key_ = std::format("{}-{}", socket_.remote_endpoint().address().to_string(), secondsSinceEpoch.count());
#endif
    }

    BaseConnection::~BaseConnection()
    {

    }

    TcpSocket& BaseConnection::socket()
    {
        return socket_;
    }

    void BaseConnection::connect()
    {
        // Mark the received timestamp
        received_ = std::chrono::steady_clock::now();

        co_spawn(socket_.get_executor(), [self = this->shared_from_this()]() -> awaitable<void> {
#ifdef URANUS_SSL
            if (const auto [ec] = co_await self->socket_.async_handshake(asio::ssl::stream_base::server); ec) {
                self->disconnect();
                co_return;
            }
#endif

            self->onConnect();

            co_await (
                self->readMessage() &&
                self->writeMessage() &&
                self->watchdog()
            );
        }, detached);
    }

    void BaseConnection::disconnect()
    {
        if (!isConnected())
            return;

#ifdef URANUS_SSL
        socket_.next_layer().close();
#else
        socket_.close();
#endif
        watchdog_.cancel();

        // if (!attr().has("REPEATED")) {
        //     server_.remove(key_);
        // }

        // Call the virtual method
        onDisconnect();
    }

    bool BaseConnection::isConnected() const
    {
#ifdef URANUS_SSL
        return socket_.next_layer().is_open();
#else
        return socket_.is_open();
#endif
    }

    const std::string& BaseConnection::getKey() const
    {
        return key_;
    }

    asio::ip::address BaseConnection::remoteAddress() const
    {
        return socket_.next_layer().remote_endpoint().address();
    }

    void BaseConnection::setExpirationSecond(const int sec)
    {
        expiration_ = std::chrono::seconds(sec);
    }

    AttributeMap& BaseConnection::attr()
    {
        return attr_;
    }

    awaitable<void> BaseConnection::watchdog()
    {
        if (expiration_ <= SteadyDuration::zero())
            co_return;

        try {
            do {
                watchdog_.expires_at(received_ + expiration_);

                if (auto [ec] = co_await watchdog_.async_wait(); ec) {
                    if (ec != asio::error::operation_aborted) {
                        // Not excepted error
                        onErrorCode(ec);
                    }

                    if (isConnected()) {
                        disconnect();
                    }

                    co_return;
                }

                onTimeout();

                if (isConnected()) {
                    disconnect();
                }
            } while (received_ + expiration_ > std::chrono::steady_clock::now());
        } catch (std::exception &e) {
            onException(e);
            disconnect();
        }
    }
}
