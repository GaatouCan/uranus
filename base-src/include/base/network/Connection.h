#pragma once

#include "Message.h"
#include "noncopy.h"

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <asio/experimental/concurrent_channel.hpp>
#include <asio/experimental/awaitable_operators.hpp>
#include <tuple>

namespace uranus::network {
    using default_token = asio::as_tuple_t<asio::use_awaitable_t<> >;
    using namespace asio::experimental::awaitable_operators;

    using asio::awaitable;
    using asio::co_spawn;
    using asio::detached;

    using std::tuple;
    using std::error_code;

    using TcpAcceptor = default_token::as_default_on_t<asio::ip::tcp::acceptor>;

#ifdef URANUS_SSL
    using TcpSocket = asio::ssl::stream<default_token::as_default_on_t<asio::ip::tcp::socket>>;
#else
    using TcpSocket = default_token::as_default_on_t<asio::ip::tcp::socket>;
#endif

    template<class T>
    using ConcurrentChannel = default_token::as_default_on_t<asio::experimental::concurrent_channel<void(error_code, T)>>;

    template<typename T>
    requires std::is_base_of_v<Message, T>
    class MessageCodec;

    template<class Codec>
    requires std::is_base_of_v<Codec, MessageCodec<typename Codec::type> >
    class Connection;

    template<typename T>
    requires std::is_base_of_v<Message, T>
    class MessageCodec {
    public:
        using type = T;

        MessageCodec() = delete;

        explicit MessageCodec(TcpSocket &&socket)
            : socket_(std::move(socket)) {
        }

        virtual ~MessageCodec() = default;

        DISABLE_COPY_MOVE(MessageCodec)

        virtual awaitable<error_code> encode(MessageHandle &&msg) = 0;
        virtual awaitable<tuple<error_code, MessageHandle> > decode() = 0;

        TcpSocket &getSocket() {
            return socket_;
        }

    private:
        TcpSocket socket_;
    };


    template<class Codec>
    requires std::is_base_of_v<Codec, MessageCodec<typename Codec::type> >
    class Connection final : public std::enable_shared_from_this<Connection<Codec> > {
    public:
        Connection() = delete;

        explicit Connection(TcpSocket &&socket)
            : codec_(std::move(socket)),
              output_(getExecutor(), 1024) {
        }

        ~Connection() {
        }

        DISABLE_COPY_MOVE(Connection)

        Codec &getCodec() {
            return codec_;
        }

        TcpSocket &getSocket() {
            return codec_.getSocket().next_layer();
        }

        [[nodiscard]] bool isConnected() const {
#ifdef URANUS_SSL
            return codec_.getSocket().next_layer().is_open();
#else
            return codec_.getSocket().is_open();
#endif
        }

        auto getExecutor() {
            return getSocket().get_executor();
        }

        void connect() {
            co_spawn(getExecutor(), [self = this->shared_from_this()]() -> awaitable<void> {
#ifdef URANUS_SSL
                const auto [ec] = co_await self->codec_.getSocket().async_handshake(asio::ssl::stream_base::server);
                if (ec) {
                    self->disconnect();
                    co_return;
                }
#endif

                co_await (
                    self->readMessage() &&
                    self->writeMessage()
                );
            }, detached);
        }

        void disconnect() {
            if (!isConnected())
                return;

#ifdef URANUS_SSL
            codec_.getSocket().next_layer().close();
#else
            codec_.getSocket().close();
#endif

            output_.cancel();
            output_.close();
        }

        void send(MessageHandle &&msg) {
            if (msg == nullptr)
                return;

            if (!isConnected())
                return;

            output_.try_send_via_dispatch(error_code{}, std::move(msg));
        }

        void send(Message *msg) {
            send({msg, Message::Deleter::make()});
        }

    private:
        awaitable<void> readMessage() {
            try {
                while (isConnected()) {
                    auto [ec, msg] = co_await codec_.decode();

                    if (ec) {
                        disconnect();
                        break;
                    }

                    // TODO: handle message
                }
            } catch (const std::exception &e) {

            }
        }

        awaitable<void> writeMessage() {
            try {
                while (isConnected() && output_.is_open()) {
                    auto [ec, msg] = co_await output_.async_receive();

                    if (ec == asio::error::operation_aborted ||
                        ec == asio::experimental::error::channel_closed) {
                        break;
                    }

                    if (msg == nullptr)
                        continue;

                    if (ec) {
                        // TODO:
                        continue;
                    }

                    if (const auto writeEc = co_await codec_.encode(std::move(msg))) {
                        // TODO
                        break;
                    }
                }
            } catch (const std::exception &e) {
            }
        }

    private:
        Codec codec_;
        ConcurrentChannel<MessageHandle> output_;
    };
}
