#include "Connection.h"

#include <chrono>
#include <format>
#include <asio/experimental/awaitable_operators.hpp>

using namespace asio::experimental::awaitable_operators;

namespace uranus::network {
    Connection::Connection(TcpSocket &&socket)
        : socket_(std::move(socket)),
          output_(socket_.get_executor(), 1024),
          pipeline_(*this),
          watchdog_(socket_.get_executor()),
          expiration_(std::chrono::seconds(30)) {

        const auto now = std::chrono::system_clock::now();
        const auto durationSinceEpoch = now.time_since_epoch();
        const auto secondsSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(durationSinceEpoch);

#ifdef URANUS_SSL
        key_ = std::format("{}-{}", socket_.next_layer().remote_endpoint().address().to_string(), secondsSinceEpoch.count());
#else
        key_ = std::format("{}-{}", socket_.remote_endpoint().address().to_string(), secondsSinceEpoch.count());
#endif
    }

    Connection::~Connection() {
        disconnect();
    }

    TcpSocket &Connection::getSocket() {
        return socket_;
    }

    MessageCodec &Connection::codec() const {
        return *codec_;
    }

    ConnectionPipeline &Connection::getPipeline() {
        return pipeline_;
    }

    void Connection::connect() {
        received_ = std::chrono::steady_clock::now();

        co_spawn(socket_.get_executor(), [self = this->shared_from_this()]() -> awaitable<void> {
#ifdef URANUS_SSL
            if (const auto [ec] = co_await self->socket_.async_handshake(asio::ssl::stream_base::server); ec) {
                self->disconnect();
                co_return;
            }
#endif

            // self->pipeline_.onConnect();

            co_await (
                self->readMessage() &&
                self->writeMessage() &&
                self->watchdog()
            );
        }, detached);
    }

    void Connection::disconnect() {
        if (!isConnected())
            return;

#ifdef URANUS_SSL
        socket_.next_layer().close();
#else
        socket_.close();
#endif

        output_.cancel();
        output_.close();

        watchdog_.cancel();
    }

    bool Connection::isConnected() const {
#ifdef URANUS_SSL
        return socket_.next_layer().is_open();
#else
        return socket_.is_open();
#endif
    }

    const std::string &Connection::getKey() const {
        return key_;
    }

    asio::ip::address Connection::remoteAddress() const {
        return socket_.next_layer().remote_endpoint().address();
    }

    void Connection::setExpirationSecond(const int sec) {
        expiration_ = std::chrono::seconds(sec);
    }

    void Connection::send(MessageHandle &&msg) {
        if (msg == nullptr)
            return;

        if (!isConnected())
            return;

        output_.try_send_via_dispatch(error_code{}, std::move(msg));
    }

    void Connection::send(Message *msg) {
        send(MessageHandle{msg, Message::Deleter::make()});
    }

    AttributeMap &Connection::attr() {
        return attr_;
    }

    awaitable<void> Connection::readMessage() {
        try {
            while (isConnected()) {
                auto [ec, msg] = co_await codec_->decode();

                if (ec) {
                    // pipeline_.onError(ec);
                    disconnect();
                    break;
                }

                received_ = std::chrono::steady_clock::now();
                // co_await pipeline_.onReceive(msg);
            }
        } catch (const std::exception &e) {
            // pipeline_.onException(e);
            disconnect();
        }
    }

    awaitable<void> Connection::writeMessage() {
        try {
            while (isConnected() && output_.is_open()) {
                auto [ec, msg] = co_await output_.async_receive();

                if (ec == asio::error::operation_aborted ||
                    ec == asio::experimental::error::channel_closed) {
                    break;
                    }

                if (ec) {
                    // pipeline_.onError(ec);
                    disconnect();
                    break;
                }

                if (msg == nullptr)
                    continue;

                // co_await pipeline_.beforeSend(msg.get());

                if (const auto writeEc = co_await codec_->encode(msg.get())) {
                    // pipeline_.onError(writeEc);
                    disconnect();
                    break;
                }

                // co_await pipeline_.afterSend(msg);
            }
        } catch (const std::exception &e) {
            // pipeline_.onException(e);
            disconnect();
        }
    }

    awaitable<void> Connection::watchdog() {
        if (expiration_ <= SteadyDuration::zero())
            co_return;

        try {
            do {
                watchdog_.expires_at(received_ + expiration_);

                if (auto [ec] = co_await watchdog_.async_wait(); ec) {
                    if (ec == asio::error::operation_aborted) {
                        // TODO
                    } else {
                        // pipeline_.onError(ec);
                    }

                    co_return;
                }

                // pipeline_.onTimeout();

                if (isConnected()) {
                    disconnect();
                }
            } while (received_ + expiration_ > std::chrono::steady_clock::now());

        } catch (const std::exception &e) {
            // pipeline_.onException(e);
            disconnect();
        }
    }
}
