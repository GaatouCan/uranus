#pragma once

#include "base/Message.h"
#include "base/noncopy.h"

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

    class ConnectionBase {

    public:
        ConnectionBase() = default;
        virtual ~ConnectionBase() = default;

        virtual TcpSocket &getSocket() = 0;

        virtual void send(MessageHandle &&msg) = 0;
    };

    template<typename T>
    requires std::is_base_of_v<Message, T>
    class MessageCodec {
    public:
        using type = T;
        using HandleType = Message::Pointer<type>;

        MessageCodec() = delete;

        explicit MessageCodec(ConnectionBase &conn);
        virtual ~MessageCodec() = default;

        DISABLE_COPY_MOVE(MessageCodec)

        virtual awaitable<error_code> encode(HandleType &&msg) = 0;
        virtual awaitable<tuple<error_code, HandleType>> decode() = 0;

        [[nodiscard]] TcpSocket &getSocket() const;

    protected:
        ConnectionBase &conn_;
    };


    template<class Codec>
    requires std::is_base_of_v<MessageCodec<typename Codec::type>, Codec>
    class Connection final : public ConnectionBase, public std::enable_shared_from_this<Connection<Codec>> {
    public:
        Connection() = delete;

        explicit Connection(TcpSocket &&socket);
        ~Connection() override;

        DISABLE_COPY_MOVE(Connection)

        Codec &getCodec();
        TcpSocket &getSocket() override;

        [[nodiscard]] bool isConnected() const;

        auto getExecutor() {
            return socket_.get_executor();
        }

        void connect();
        void disconnect();

        void send(MessageHandle &&msg) override;
        void send(Message *msg);

    private:
        awaitable<void> readMessage();
        awaitable<void> writeMessage();

    private:
        TcpSocket socket_;
        Codec codec_;
        ConcurrentChannel<MessageHandle> output_;
    };

    template<typename T>
    requires std::is_base_of_v<Message, T>
    MessageCodec<T>::MessageCodec(ConnectionBase &conn)
        : conn_(conn){

    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    TcpSocket &MessageCodec<T>::getSocket() const {
        return conn_.getSocket();
    }

    template<class Codec>
    requires std::is_base_of_v<MessageCodec<typename Codec::type>, Codec>
    Connection<Codec>::Connection(TcpSocket &&socket)
        : socket_(std::move(socket)),
          codec_(*this),
          output_(socket_.get_executor(), 1024) {
    }

    template<class Codec>
    requires std::is_base_of_v<MessageCodec<typename Codec::type>, Codec>
    Connection<Codec>::~Connection() {
    }

    template<class Codec>
    requires std::is_base_of_v<MessageCodec<typename Codec::type>, Codec>
    Codec &Connection<Codec>::getCodec() {
        return codec_;
    }

    template<class Codec>
    requires std::is_base_of_v<MessageCodec<typename Codec::type>, Codec>
    TcpSocket &Connection<Codec>::getSocket() {
        return socket_;
    }

    template<class Codec>
    requires std::is_base_of_v<MessageCodec<typename Codec::type>, Codec>
    bool Connection<Codec>::isConnected() const {
#ifdef URANUS_SSL
        return socket_.next_layer().is_open();
#else
        return socket_.is_open();
#endif
    }

    template<class Codec>
    requires std::is_base_of_v<MessageCodec<typename Codec::type>, Codec>
    void Connection<Codec>::connect() {
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

    template<class Codec>
    requires std::is_base_of_v<MessageCodec<typename Codec::type>, Codec>
    void Connection<Codec>::disconnect() {
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

    template<class Codec>
    requires std::is_base_of_v<MessageCodec<typename Codec::type>, Codec>
    void Connection<Codec>::send(MessageHandle &&msg) {
        if (msg == nullptr)
            return;

        if (!isConnected())
            return;

        output_.try_send_via_dispatch(error_code{}, std::move(msg));
    }

    template<class Codec>
    requires std::is_base_of_v<MessageCodec<typename Codec::type>, Codec>
    void Connection<Codec>::send(Message *msg) {
        send({msg, Message::Deleter::make()});
    }

    template<class Codec>
    requires std::is_base_of_v<MessageCodec<typename Codec::type>, Codec>
    awaitable<void> Connection<Codec>::readMessage() {
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

    template<class Codec>
    requires std::is_base_of_v<MessageCodec<typename Codec::type>, Codec>
    awaitable<void> Connection<Codec>::writeMessage() {
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
}
