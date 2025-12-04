#pragma once

#include "Message.h"
#include "noncopy.h"
#include "types.h"
#include "AttributeMap.h"

#include <tuple>
#include <string>
#include <asio/detached.hpp>
#include <asio/experimental/awaitable_operators.hpp>


namespace uranus {

    using asio::awaitable;
    using asio::co_spawn;
    using asio::detached;

    using std::tuple;
    using std::error_code;
    using std::shared_ptr;
    using std::make_shared;
    using std::enable_shared_from_this;

    class BASE_API Connection {

    public:
        Connection() = delete;

        explicit Connection(TcpSocket &&socket);
        virtual ~Connection();

        DISABLE_COPY_MOVE(Connection)

        TcpSocket &getSocket();

        virtual void connect() = 0;
        virtual void disconnect() = 0;

        [[nodiscard]] bool isConnected() const;
        [[nodiscard]] const std::string &getKey() const;

        virtual void sendMessage(MessageHandle &&msg) = 0;
        virtual void sendMessage(Message *msg) = 0;

        AttributeMap &attr();

    protected:
        TcpSocket socket_;

    private:
        std::string key_;
        AttributeMap attr_;

        SteadyTimer watchdog_;
    };

    template<typename T>
    requires std::is_base_of_v<Message, T>
    class MessageCodec {

    public:
        using Type = T;
        using HandleType = Message::Pointer<Type>;

        MessageCodec() = delete;

        explicit MessageCodec(Connection &conn);
        virtual ~MessageCodec() = default;

        DISABLE_COPY_MOVE(MessageCodec)

        virtual awaitable<error_code> encode(HandleType &&msg) = 0;
        virtual awaitable<tuple<error_code, HandleType>> decode() = 0;

        [[nodiscard]] Connection &getConnection() const;
        [[nodiscard]] TcpSocket &getSocket() const;

    protected:
        Connection &conn_;
    };


    template<typename T>
    requires std::is_base_of_v<Message, T>
    class ConnectionHandler {

    public:
        using Type = T;
        using HandleType = Message::Pointer<Type>;

        ConnectionHandler() = delete;

        explicit ConnectionHandler(Connection &conn);
        virtual ~ConnectionHandler();

        DISABLE_COPY_MOVE(ConnectionHandler)

        [[nodiscard]] Connection &getConnection() const;

        virtual void onConnect() {}
        virtual void onDisconnect() {}

        virtual void onError(error_code ec) {}
        virtual void onException(const std::exception &e) {}

        virtual void onReceive(HandleType &&msg) = 0;
        virtual void onWrite(Type *msg) = 0;

    protected:
        Connection &conn_;
    };

    namespace detail {

        template<class Codec, class Handler>
        concept ConnectionConcept = std::is_base_of_v<MessageCodec<typename Codec::Type>, Codec> &&
                     std::is_base_of_v<ConnectionHandler<typename Handler::Type>, Handler> &&
                     std::is_same_v<typename Codec::Type, typename Handler::Type>;


        template<class Codec, class Handler>
        requires ConnectionConcept<Codec, Handler>
        class ConnectionImpl final : public Connection, public enable_shared_from_this<ConnectionImpl<Codec, Handler>> {

        public:
            using MessageType = Codec::Type;
            using MessageHandleType = Codec::HandleType;

            ConnectionImpl() = delete;

            explicit ConnectionImpl(TcpSocket &&socket);
            ~ConnectionImpl() override;

            DISABLE_COPY_MOVE(ConnectionImpl)

            Codec &getCodec();
            Handler &getHandler();

            void connect() override;
            void disconnect() override;

            void send(MessageHandleType &&msg);
            void send(MessageType *msg);

            void sendMessage(MessageHandle &&msg) override;
            void sendMessage(Message *msg) override;

        private:
            awaitable<void> readMessage();
            awaitable<void> writeMessage();

        private:
            Codec codec_;
            Handler handler_;

            ConcurrentChannel<MessageHandleType> output_;
        };
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    MessageCodec<T>::MessageCodec(Connection &conn)
        : conn_(conn) {
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    Connection &MessageCodec<T>::getConnection() const {
        return conn_;
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    TcpSocket &MessageCodec<T>::getSocket() const {
        return conn_.getSocket();
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    ConnectionHandler<T>::ConnectionHandler(Connection &conn)
        : conn_(conn) {
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    ConnectionHandler<T>::~ConnectionHandler() = default;

    template<typename T>
    requires std::is_base_of_v<Message, T>
    Connection &ConnectionHandler<T>::getConnection() const {
        return conn_;
    }

    namespace detail {

        using namespace asio::experimental::awaitable_operators;

        template<class Codec, class Handler>
        requires ConnectionConcept<Codec, Handler>
        ConnectionImpl<Codec, Handler>::ConnectionImpl(TcpSocket &&socket)
            : Connection(std::move(socket)),
              codec_(*this),
              handler_(*this),
              output_(socket_.get_executor(), 1024) {

            }

        template<class Codec, class Handler>
        requires ConnectionConcept<Codec, Handler>
        ConnectionImpl<Codec, Handler>::~ConnectionImpl() {
            ConnectionImpl::disconnect();
        }

        template<class Codec, class Handler>
        requires ConnectionConcept<Codec, Handler>
        Codec &ConnectionImpl<Codec, Handler>::getCodec() {
            return codec_;
        }

        template<class Codec, class Handler>
        requires ConnectionConcept<Codec, Handler>
        Handler &ConnectionImpl<Codec, Handler>::getHandler() {
            return handler_;
        }

        template<class Codec, class Handler>
        requires ConnectionConcept<Codec, Handler>
        void ConnectionImpl<Codec, Handler>::connect() {
            co_spawn(socket_.get_executor(), [self = this->shared_from_this()]() -> awaitable<void> {
#ifdef URANUS_SSL
                if (const auto [ec] = co_await self->socket_.async_handshake(asio::ssl::stream_base::server); ec) {
                    self->disconnect();
                    co_return;
                }
#endif

                self->handler_.onConnect();

                co_await (
                    self->readMessage() &&
                    self->writeMessage()
                );
            }, detached);
        }

        template<class Codec, class Handler>
        requires ConnectionConcept<Codec, Handler>
        void ConnectionImpl<Codec, Handler>::disconnect() {
            if (!isConnected())
                return;

#ifdef URANUS_SSL
            codec_.getSocket().next_layer().close();
#else
            codec_.getSocket().close();
#endif

            output_.cancel();
            output_.close();

            handler_.onDisconnect();
        }

        template<class Codec, class Handler>
        requires ConnectionConcept<Codec, Handler>
        void ConnectionImpl<Codec, Handler>::send(MessageHandleType &&msg) {
            if (msg == nullptr)
                return;

            if (!isConnected())
                return;

            output_.try_send_via_dispatch(error_code{}, std::move(msg));
        }

        template<class Codec, class Handler>
        requires ConnectionConcept<Codec, Handler>
        void ConnectionImpl<Codec, Handler>::send(MessageType *msg) {
            send({msg, Message::Deleter::make()});
        }

        template<class Codec, class Handler>
        requires ConnectionConcept<Codec, Handler>
        void ConnectionImpl<Codec, Handler>::sendMessage(MessageHandle &&msg) {
            if (msg == nullptr)
                return;

            auto del = msg.get_deleter();
            auto *ptr = msg.release();

            if (auto *temp = dynamic_cast<MessageType *>(ptr)) {
                MessageHandleType handle{ temp, del };
                send(std::move(handle));
                return;
            }

            del(ptr);
        }

        template<class Codec, class Handler>
        requires ConnectionConcept<Codec, Handler>
        void ConnectionImpl<Codec, Handler>::sendMessage(Message *msg) {
            if (msg == nullptr)
                return;

            if (auto *temp = dynamic_cast<MessageType *>(msg)) {
                send(temp);
            }
        }

        template<class Codec, class Handler>
        requires ConnectionConcept<Codec, Handler>
        awaitable<void> ConnectionImpl<Codec, Handler>::readMessage() {
            try {
                while (isConnected()) {
                    auto [ec, msg] = co_await codec_.decode();

                    if (ec) {
                        handler_.onError(ec);
                        disconnect();
                        break;
                    }

                    handler_.onReceive(std::move(msg));
                }
            } catch (const std::exception &e) {
                handler_.onException(e);
                disconnect();
            }
        }

        template<class Codec, class Handler>
        requires ConnectionConcept<Codec, Handler>
        awaitable<void> ConnectionImpl<Codec, Handler>::writeMessage() {
            try {
                while (isConnected() && output_.is_open()) {
                    auto [ec, msg] = co_await output_.async_receive();

                    if (ec == asio::error::operation_aborted ||
                        ec == asio::experimental::error::channel_closed) {
                        break;
                    }

                    if (ec) {
                        handler_.onError(ec);
                        disconnect();
                        break;
                    }

                    if (msg == nullptr)
                        continue;

                    if (ec) {
                        handler_.onError(ec);
                        continue;
                    }

                    if (const auto writeEc = co_await codec_.encode(std::move(msg))) {
                        handler_.onError(ec);
                        disconnect();
                        break;
                    }
                }
            } catch (const std::exception &e) {
                handler_.onException(e);
                disconnect();
            }
        }
    }

    template<class Codec, class Handler>
    requires detail::ConnectionConcept<Codec, Handler>
    shared_ptr<detail::ConnectionImpl<Codec, Handler>> MakeConnection(TcpSocket &&socket) {
        return make_shared<detail::ConnectionImpl<Codec, Handler>>(std::move(socket));
    }
}
