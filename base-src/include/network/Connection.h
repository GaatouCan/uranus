#pragma once

#include "Message.h"
#include "noncopy.h"
#include "types.h"
#include "AttributeMap.h"
#include "ConnectionPipeline.h"

#include <tuple>
#include <string>
#include <asio/detached.hpp>
#include <asio/experimental/awaitable_operators.hpp>


namespace uranus::network {

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

        [[nodiscard]] asio::ip::address remoteAddress() const;

        void setExpirationSecond(int sec);

        virtual void sendMessage(MessageHandle &&msg) = 0;
        virtual void sendMessage(Message *msg) = 0;

        AttributeMap &attr();
        ConnectionPipeline &pipeline();

    protected:
        TcpSocket socket_;
        ConnectionPipeline pipeline_;

        std::string key_;
        AttributeMap attr_;

        SteadyTimer watchdog_;
        SteadyDuration expiration_;
        SteadyTimePoint received_;
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

    namespace detail {

        template<class Codec>
        requires std::is_base_of_v<MessageCodec<typename Codec::Type>, Codec>
        class ConnectionImpl final : public Connection, public enable_shared_from_this<ConnectionImpl<Codec>> {

        public:
            using MessageType = Codec::Type;
            using MessageHandleType = Codec::HandleType;

            ConnectionImpl() = delete;

            explicit ConnectionImpl(TcpSocket &&socket);
            ~ConnectionImpl() override;

            DISABLE_COPY_MOVE(ConnectionImpl)

            Codec &getCodec();

            void connect() override;
            void disconnect() override;

            void send(MessageHandleType &&msg);
            void send(MessageType *msg);

            void sendMessage(MessageHandle &&msg) override;
            void sendMessage(Message *msg) override;

        private:
            awaitable<void> readMessage();
            awaitable<void> writeMessage();
            awaitable<void> watchdog();
        
        private:
            Codec codec_;
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

     // template<typename T>
     // requires std::is_base_of_v<Message, T>
     // ConnectionHandler<T>::ConnectionHandler(Connection &conn)
     //     : conn_(conn) {
     // }
     //
     // template<typename T>
     // requires std::is_base_of_v<Message, T>
     // ConnectionHandler<T>::~ConnectionHandler() = default;
     //
     // template<typename T>
     // requires std::is_base_of_v<Message, T>
     // Connection &ConnectionHandler<T>::getConnection() const {
     //     return conn_;
     // }
     //
     // template<typename T>
     // requires std::is_base_of_v<Message, T>
     // const std::string &ConnectionHandler<T>::getKey() const {
     //     return conn_.getKey();
     // }
     //
     // template<typename T>
     // requires std::is_base_of_v<Message, T>
     // asio::ip::address ConnectionHandler<T>::remoteAddress() const {
     //     return conn_.remoteAddress();
     // }
     //
     // template<typename T>
     // requires std::is_base_of_v<Message, T>
     // AttributeMap &ConnectionHandler<T>::attr() const {
     //     return conn_.attr();
     // }
     //
     // template<typename T>
     // requires std::is_base_of_v<Message, T>
     // void ConnectionHandler<T>::sendMessage(MessageHandle &&msg) const {
     //     conn_.sendMessage(std::move(msg));
     // }
     //
     // template<typename T>
     // requires std::is_base_of_v<Message, T>
     // void ConnectionHandler<T>::sendMessage(Message *msg) const {
     //     conn_.sendMessage(msg);
     // }
//
    namespace detail {

        using namespace asio::experimental::awaitable_operators;

        template<class Codec>
        requires std::is_base_of_v<MessageCodec<typename Codec::Type>, Codec>
        ConnectionImpl<Codec>::ConnectionImpl(TcpSocket &&socket)
            : Connection(std::move(socket)),
              codec_(*this),
              output_(socket_.get_executor(), 1024) {

            }

        template<class Codec>
        requires std::is_base_of_v<MessageCodec<typename Codec::Type>, Codec>
        ConnectionImpl<Codec>::~ConnectionImpl() {
            ConnectionImpl::disconnect();
        }

        template<class Codec>
        requires std::is_base_of_v<MessageCodec<typename Codec::Type>, Codec>
        Codec &ConnectionImpl<Codec>::getCodec() {
            return codec_;
        }

        template<class Codec>
        requires std::is_base_of_v<MessageCodec<typename Codec::Type>, Codec>
        void ConnectionImpl<Codec>::connect() {

            received_ = std::chrono::steady_clock::now();

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
                    self->writeMessage() &&
                    self->watchdog()
                );
            }, detached);
        }

        template<class Codec>
        requires std::is_base_of_v<MessageCodec<typename Codec::Type>, Codec>
        void ConnectionImpl<Codec>::disconnect() {
            if (!isConnected())
                return;

#ifdef URANUS_SSL
            codec_.getSocket().next_layer().close();
#else
            codec_.getSocket().close();
#endif

            output_.cancel();
            output_.close();

            // handler_.onDisconnect();
        }

        template<class Codec>
        requires std::is_base_of_v<MessageCodec<typename Codec::Type>, Codec>
        void ConnectionImpl<Codec>::send(MessageHandleType &&msg) {
            if (msg == nullptr)
                return;

            if (!isConnected())
                return;

            output_.try_send_via_dispatch(error_code{}, std::move(msg));
        }

        template<class Codec>
        requires std::is_base_of_v<MessageCodec<typename Codec::Type>, Codec>
        void ConnectionImpl<Codec>::send(MessageType *msg) {
            send({msg, Message::Deleter::make()});
        }

        template<class Codec>
        requires std::is_base_of_v<MessageCodec<typename Codec::Type>, Codec>
        void ConnectionImpl<Codec>::sendMessage(MessageHandle &&msg) {
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

        template<class Codec>
        requires std::is_base_of_v<MessageCodec<typename Codec::Type>, Codec>
        void ConnectionImpl<Codec>::sendMessage(Message *msg) {
            if (msg == nullptr)
                return;

            if (auto *temp = dynamic_cast<MessageType *>(msg)) {
                send(temp);
            }
        }

        template<class Codec>
        requires std::is_base_of_v<MessageCodec<typename Codec::Type>, Codec>
        awaitable<void> ConnectionImpl<Codec>::readMessage() {
            try {
                while (isConnected()) {
                    auto [ec, msg] = co_await codec_.decode();

                    if (ec) {
                        //handler_.onError(ec);
                        disconnect();
                        break;
                    }

                    received_ = std::chrono::steady_clock::now();
                    pipeline_.onReceive(std::move(msg));
                }
            } catch (const std::exception &e) {
                // handler_.onException(e);
                disconnect();
            }
        }

        template<class Codec>
        requires std::is_base_of_v<MessageCodec<typename Codec::Type>, Codec>
        awaitable<void> ConnectionImpl<Codec>::writeMessage() {
            try {
                while (isConnected() && output_.is_open()) {
                    auto [ec, msg] = co_await output_.async_receive();

                    if (ec == asio::error::operation_aborted ||
                        ec == asio::experimental::error::channel_closed) {
                        break;
                    }

                    if (ec) {
                        //handler_.onError(ec);
                        disconnect();
                        break;
                    }

                    if (msg == nullptr)
                        continue;

                    if (ec) {
                        //handler_.onError(ec);
                        continue;
                    }

                    if (const auto writeEc = co_await codec_.encode(std::move(msg))) {
                        //handler_.onError(ec);
                        disconnect();
                        break;
                    }
                }
            } catch (const std::exception &e) {
                // handler_.onException(e);
                disconnect();
            }
        }

        template<class Codec>
        requires std::is_base_of_v<MessageCodec<typename Codec::Type>, Codec>
        awaitable<void> ConnectionImpl<Codec>::watchdog() {
            if (expiration_ <= SteadyDuration::zero())
                co_return;

            try {
                do {
                    watchdog_.expires_at(received_ + expiration_);

                    if (auto [ec] = co_await watchdog_.async_wait(); ec) {
                        if (ec == asio::error::operation_aborted) {
                            // TODO
                        }
                        else {
                            // handler_.onError(ec);
                        }

                        co_return;
                    }

                    // handler_.onTimeout();

                    if (isConnected()) {
                        disconnect();
                    }
                } while (received_ + expiration_ > std::chrono::steady_clock::now());
            } catch (const std::exception &e) {
                //handler_.onException(e);
            }
        }
    }

     template<class Codec, class Handler>
     requires std::is_base_of_v<MessageCodec<typename Codec::Type>, Codec>
     shared_ptr<detail::ConnectionImpl<Codec>> MakeConnection(TcpSocket &&socket) {
         return make_shared<detail::ConnectionImpl<Codec>>(std::move(socket));
     }
}
