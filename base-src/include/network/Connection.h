#pragma once

#include "base/Message.h"
#include "base/noncopy.h"
#include "base/types.h"
#include "base/AttributeMap.h"

#include <tuple>
#include <string>
#include <vector>
#include <asio/detached.hpp>
#include <asio/experimental/awaitable_operators.hpp>


namespace uranus::network {

    using asio::awaitable;
    using asio::co_spawn;
    using asio::detached;

    using std::tuple;
    using std::vector;
    using std::error_code;
    using std::shared_ptr;
    using std::unique_ptr;
    using std::make_tuple;
    using std::make_shared;
    using std::make_unique;
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

    protected:
        TcpSocket socket_;

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

        virtual awaitable<error_code> encode(Type *msg) = 0;
        virtual awaitable<tuple<error_code, HandleType> > decode() = 0;

        [[nodiscard]] Connection &getConnection() const;
        [[nodiscard]] TcpSocket &getSocket() const;

    protected:
        Connection &conn_;
    };

    class BASE_API ConnectionPipelineContext final {

    public:
        ConnectionPipelineContext() = delete;

        explicit ConnectionPipelineContext(Connection &conn);
        ~ConnectionPipelineContext();

        [[nodiscard]] Connection &getConnection() const;
        [[nodiscard]] AttributeMap &attr() const;

        void fire();
        [[nodiscard]] bool fired() const;

    private:
        Connection &conn_;
        bool fire_;
    };

    template<typename T>
    requires std::is_base_of_v<Message, T>
    class ConnectionHandler {
    public:
        using MessageType = T;
        using MessageHandleType = Message::Pointer<T>;

        ConnectionHandler();
        virtual ~ConnectionHandler() = default;
    };


    template<typename T>
    requires std::is_base_of_v<Message, T>
    class ConnectionInboundHandler : virtual public ConnectionHandler<T> {

        public:
        using MessageType = ConnectionHandler<T>::MessageType;
        using MessageHandleType = ConnectionHandler<T>::MessageHandleType;

        ConnectionInboundHandler() = default;
        ~ConnectionInboundHandler() override = default;

        virtual void onConnect(ConnectionPipelineContext &ctx);
        virtual void onDisconnect(ConnectionPipelineContext &ctx);

        virtual awaitable<void> onReceive(ConnectionPipelineContext &ctx, MessageHandleType &ref) = 0;

        virtual void onError(ConnectionPipelineContext &ctx, std::error_code ec);
        virtual void onException(ConnectionPipelineContext &ctx, const std::exception &e);

        virtual void onTimeout(ConnectionPipelineContext &ctx);
    };

    template<typename T>
    requires std::is_base_of_v<Message, T>
    class ConnectionOutboundHandler : virtual public ConnectionHandler<T> {
    public:
        using MessageType = ConnectionHandler<T>::MessageType;
        using MessageHandleType = ConnectionHandler<T>::MessageHandleType;

        ConnectionOutboundHandler() = default;
        ~ConnectionOutboundHandler() override = default;

        virtual awaitable<void> beforeSend(ConnectionPipelineContext &ctx, MessageType *msg);
        virtual awaitable<void> afterSend(ConnectionPipelineContext &ctx, MessageHandleType &ref);
    };

    namespace detail {
        template<class Msg, class T>
        concept kHandlerType = std::is_base_of_v<ConnectionHandler<Msg>, T>;

        template<class Msg, class T>
        concept kCodecType = std::is_base_of_v<MessageCodec<Msg>, T>;

        template<class Codec, class... Handlers>
        concept kConnectionConcept = kCodecType<typename Codec::Type, Codec> && (kHandlerType<typename Codec::Type, Handlers> && ...);

        template<class Msg, class... Handlers>
        concept kPipelineConcept = (kHandlerType<Msg, Handlers> && ...);

        template<class Msg, class ...handlers>
        requires kPipelineConcept<Msg, handlers...>
        class ConnectionPipeline final {
        public:
            using MessageType = Msg;
            using MessageHandleType = Message::Pointer<Msg>;

            ConnectionPipeline() = delete;

            explicit ConnectionPipeline(Connection &conn);
            ~ConnectionPipeline();

            DISABLE_COPY_MOVE(ConnectionPipeline)

            [[nodiscard]] Connection &getConnection() const;

            template<size_t idx>
            auto &getHandler();

            template<size_t index = 0>
            void onConnect();

            template<size_t index = 0>
            void onDisconnect();

            template<size_t index = 0>
            awaitable<void> onReceive(MessageHandleType &msg);

            template<size_t index = sizeof...(handlers)>
            awaitable<void> beforeSend(MessageType *msg);

            template<size_t index = sizeof...(handlers)>
            awaitable<void> afterSend(MessageHandleType &msg);

            template<size_t index = 0>
            void onError(std::error_code ec);

            template<size_t index = 0>
            void onException(const std::exception &e);

            template<size_t index = 0>
            void onTimeout();

        private:
            Connection &conn_;
            tuple<handlers...> handlers_;
        };


        template<class Codec, class ...handlers>
        requires kConnectionConcept<Codec, handlers...>
        class ConnectionImpl final : public Connection, public enable_shared_from_this<ConnectionImpl<Codec, handlers...>> {
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

            template<size_t idx>
            auto &getHandler();

        private:
            awaitable<void> readMessage();
            awaitable<void> writeMessage();
            awaitable<void> watchdog();

        private:
            Codec codec_;
            ConcurrentChannel<MessageHandleType> output_;
            ConnectionPipeline<MessageType, handlers...> pipeline_;
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
    ConnectionHandler<T>::ConnectionHandler() = default;

    namespace detail {

        template<class Msg, class T>
        constexpr bool kCheckInbound = std::is_base_of_v<ConnectionInboundHandler<Msg>, std::decay_t<T>>;

        template<class Msg, class T>
        constexpr bool kCheckOutbound = std::is_base_of_v<ConnectionOutboundHandler<Msg>, std::decay_t<T>>;

        template<class Msg, class ...handlers>
        requires kPipelineConcept<Msg, handlers...>
        ConnectionPipeline<Msg, handlers...>::ConnectionPipeline(Connection &conn)
            : conn_(conn) {
        }

        template<class Msg, class ...handlers>
        requires kPipelineConcept<Msg, handlers...>
        ConnectionPipeline<Msg, handlers...>::~ConnectionPipeline() = default;

        template<class Msg, class ...handlers>
        requires kPipelineConcept<Msg, handlers...>
        Connection &ConnectionPipeline<Msg, handlers...>::getConnection() const {
            return conn_;
        }

        template<class Msg, class ...handlers>
        requires kPipelineConcept<Msg, handlers...>
        template<size_t idx>
        auto &ConnectionPipeline<Msg, handlers...>::getHandler() {
            return std::get<idx>(handlers_);
        }

        template<class Msg, class ...handlers>
        requires kPipelineConcept<Msg, handlers...>
        template<size_t index>
        void ConnectionPipeline<Msg, handlers...>::onConnect() {
            if constexpr (index < sizeof...(handlers)) {
                if constexpr (auto &handler = std::get<index>(handlers_); kCheckInbound<Msg, decltype(handler)>) {
                    ConnectionPipelineContext ctx(conn_);
                    handler.onConnect(ctx);

                    if (!ctx.fired())
                        return;
                }

                onConnect<index + 1>();
            }
        }

        template<class Msg, class ...handlers>
        requires kPipelineConcept<Msg, handlers...>
        template<size_t index>
        void ConnectionPipeline<Msg, handlers...>::onDisconnect() {
            if constexpr (index < sizeof...(handlers)) {
                if constexpr (auto &handler = std::get<index>(handlers_); kCheckInbound<Msg, decltype(handler)>) {
                    ConnectionPipelineContext ctx(conn_);
                    handler.onDisconnect(ctx);

                    if (!ctx.fired())
                        return;
                }

                onDisconnect<index + 1>();
            }
        }

        template<class Msg, class ...handlers>
        requires kPipelineConcept<Msg, handlers...>
        template<size_t index>
        awaitable<void> ConnectionPipeline<Msg, handlers...>::onReceive(MessageHandleType &msg) {
            if constexpr (index < sizeof...(handlers)) {
                if constexpr (auto &handler = std::get<index>(handlers_); kCheckInbound<Msg, decltype(handler)>) {
                    ConnectionPipelineContext ctx(conn_);
                    co_await handler.onReceive(ctx, msg);

                    if (!ctx.fired() || msg == nullptr)
                        co_return;
                }

                co_await onReceive<index + 1>(msg);
            }

            co_return;
        }

        template<class Msg, class ...handlers>
        requires kPipelineConcept<Msg, handlers...>
        template<size_t index>
        awaitable<void> ConnectionPipeline<Msg, handlers...>::beforeSend(MessageType *msg) {
            if constexpr (index > 0) {
                if constexpr (auto &handler = std::get<index - 1>(handlers_); kCheckOutbound<Msg, decltype(handler)>) {
                    ConnectionPipelineContext ctx(conn_);
                    handler.beforeSend(ctx, msg);

                    if (ctx.fired())
                        co_return;
                }

                co_await beforeSend<index - 1>(msg);
            }

            co_return;
        }

        template<class Msg, class ...handlers>
        requires kPipelineConcept<Msg, handlers...>
        template<size_t index>
        awaitable<void> ConnectionPipeline<Msg, handlers...>::afterSend(MessageHandleType &msg) {
            if constexpr (index > 0) {
                if constexpr (auto &handler = std::get<index - 1>(handlers_); kCheckOutbound<Msg, decltype(handler)>) {
                    ConnectionPipelineContext ctx(conn_);
                    handler.afterSend(ctx, msg);

                    if (ctx.fired())
                        co_return;
                }

                co_await afterSend<index - 1>(msg);
            }

            co_return;
        }

        template<class Msg, class ...handlers>
        requires kPipelineConcept<Msg, handlers...>
        template<size_t index>
        void ConnectionPipeline<Msg, handlers...>::onError(std::error_code ec) {
            if constexpr (index < sizeof...(handlers)) {
                if constexpr (auto &handler = std::get<index>(handlers_); kCheckInbound<Msg, decltype(handler)>) {
                    ConnectionPipelineContext ctx(conn_);
                    handler.onError(ctx, ec);

                    if (!ctx.fired())
                        return;
                }

                onError<index + 1>(ec);
            }
        }

        template<class Msg, class ...handlers>
        requires kPipelineConcept<Msg, handlers...>
        template<size_t index>
        void ConnectionPipeline<Msg, handlers...>::onException(const std::exception &e) {
            if constexpr (index < sizeof...(handlers)) {
                if constexpr (auto &handler = std::get<index>(handlers_); kCheckInbound<Msg, decltype(handler)>) {
                    ConnectionPipelineContext ctx(conn_);
                    handler.onException(ctx, e);

                    if (!ctx.fired())
                        return;
                }

                onException<index + 1>(e);
            }
        }

        template<class Msg, class ...handlers>
        requires kPipelineConcept<Msg, handlers...>
        template<size_t index>
        void ConnectionPipeline<Msg, handlers...>::onTimeout() {
            if constexpr (index < sizeof...(handlers)) {
                if constexpr (auto &handler = std::get<index>(handlers_); kCheckInbound<Msg, decltype(handler)>) {
                    ConnectionPipelineContext ctx(conn_);
                    handler.onTimeout(ctx);

                    if (!ctx.fired())
                        return;
                }

                onTimeout<index + 1>();
            }
        }
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    void ConnectionInboundHandler<T>::onConnect(ConnectionPipelineContext &ctx) {
        ctx.fire();
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    void ConnectionInboundHandler<T>::onDisconnect(ConnectionPipelineContext &ctx) {
        ctx.fire();
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    void ConnectionInboundHandler<T>::onTimeout(ConnectionPipelineContext &ctx) {
        ctx.fire();
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    void ConnectionInboundHandler<T>::onError(ConnectionPipelineContext &ctx, std::error_code ec) {
        ctx.fire();
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    void ConnectionInboundHandler<T>::onException(ConnectionPipelineContext &ctx, const std::exception &e) {
        ctx.fire();
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    awaitable<void> ConnectionOutboundHandler<T>::beforeSend(ConnectionPipelineContext &ctx, MessageType *msg) {
        ctx.fire();
        co_return;
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    awaitable<void> ConnectionOutboundHandler<T>::afterSend(ConnectionPipelineContext &ctx, MessageHandleType &ref) {
        ctx.fire();
        co_return;
    }


    namespace detail {
        using namespace asio::experimental::awaitable_operators;

        template<class Codec, class ...handlers>
        requires kConnectionConcept<Codec, handlers...>
        ConnectionImpl<Codec, handlers...>::ConnectionImpl(TcpSocket &&socket)
            : Connection(std::move(socket)),
              codec_(*this),
              output_(socket_.get_executor(), 1024),
              pipeline_(*this) {
        }

        template<class Codec, class ...handlers>
        requires kConnectionConcept<Codec, handlers...>
        ConnectionImpl<Codec, handlers...>::~ConnectionImpl() {
            ConnectionImpl::disconnect();
        }

        template<class Codec, class ...handlers>
        requires kConnectionConcept<Codec, handlers...>
        Codec &ConnectionImpl<Codec, handlers...>::getCodec() {
            return codec_;
        }

        template<class Codec, class ...handlers>
        requires kConnectionConcept<Codec, handlers...>
        void ConnectionImpl<Codec, handlers...>::connect() {
            received_ = std::chrono::steady_clock::now();

            co_spawn(socket_.get_executor(), [self = this->shared_from_this(), this]() -> awaitable<void> {
#ifdef URANUS_SSL
                if (const auto [ec] = co_await socket_.async_handshake(asio::ssl::stream_base::server); ec) {
                    disconnect();
                    co_return;
                }
#endif

                pipeline_.onConnect();

                co_await (
                    readMessage() &&
                    writeMessage() &&
                    watchdog()
                );
            }, detached);
        }

        template<class Codec, class ...handlers>
        requires kConnectionConcept<Codec, handlers...>
        void ConnectionImpl<Codec, handlers...>::disconnect() {
            if (!isConnected())
                return;

#ifdef URANUS_SSL
            codec_.getSocket().next_layer().close();
#else
            codec_.getSocket().close();
#endif

            output_.cancel();
            output_.close();

            watchdog_.cancel();

            pipeline_.onDisconnect();
        }

        template<class Codec, class ...handlers>
        requires kConnectionConcept<Codec, handlers...>
        void ConnectionImpl<Codec, handlers...>::send(MessageHandleType &&msg) {
            if (msg == nullptr)
                return;

            if (!isConnected())
                return;

            output_.try_send_via_dispatch(error_code{}, std::move(msg));
        }

        template<class Codec, class ...handlers>
        requires kConnectionConcept<Codec, handlers...>
        void ConnectionImpl<Codec, handlers...>::send(MessageType *msg) {
            send({msg, Message::Deleter::make()});
        }

        template<class Codec, class ...handlers>
        requires kConnectionConcept<Codec, handlers...>
        void ConnectionImpl<Codec, handlers...>::sendMessage(MessageHandle &&msg) {
            if (msg == nullptr)
                return;

            auto del = msg.get_deleter();
            auto *ptr = msg.release();

            if (auto *temp = dynamic_cast<MessageType *>(ptr)) {
                MessageHandleType handle{temp, del};
                send(std::move(handle));
                return;
            }

            del(ptr);
        }

        template<class Codec, class ...handlers>
        requires kConnectionConcept<Codec, handlers...>
        void ConnectionImpl<Codec, handlers...>::sendMessage(Message *msg) {
            if (msg == nullptr)
                return;

            if (auto *temp = dynamic_cast<MessageType *>(msg)) {
                send(temp);
            }
        }

        template<class Codec, class ...handlers>
        requires kConnectionConcept<Codec, handlers...>
        template<size_t idx>
        auto &ConnectionImpl<Codec, handlers...>::getHandler() {
            return pipeline_.template getHandler<idx>();
        }

        template<class Codec, class ...handlers>
        requires kConnectionConcept<Codec, handlers...>
        awaitable<void> ConnectionImpl<Codec, handlers...>::readMessage() {
            try {
                while (isConnected()) {
                    auto [ec, msg] = co_await codec_.decode();

                    if (ec) {
                        pipeline_.onError(ec);
                        disconnect();
                        break;
                    }

                    received_ = std::chrono::steady_clock::now();
                    co_await pipeline_.onReceive(msg);
                }
            } catch (const std::exception &e) {
                pipeline_.onException(e);
                disconnect();
            }
        }

        template<class Codec, class ...handlers>
        requires kConnectionConcept<Codec, handlers...>
        awaitable<void> ConnectionImpl<Codec, handlers...>::writeMessage() {
            try {
                while (isConnected() && output_.is_open()) {
                    auto [ec, msg] = co_await output_.async_receive();

                    if (ec == asio::error::operation_aborted ||
                        ec == asio::experimental::error::channel_closed) {
                        break;
                    }

                    if (ec) {
                        pipeline_.onError(ec);
                        disconnect();
                        break;
                    }

                    if (msg == nullptr)
                        continue;

                    co_await pipeline_.beforeSend(msg.get());

                    if (const auto writeEc = co_await codec_.encode(msg.get())) {
                        pipeline_.onError(writeEc);
                        disconnect();
                        break;
                    }

                    co_await pipeline_.afterSend(msg);
                }
            } catch (const std::exception &e) {
                pipeline_.onException(e);
                disconnect();
            }
        }

        template<class Codec, class ...handlers>
        requires kConnectionConcept<Codec, handlers...>
        awaitable<void> ConnectionImpl<Codec, handlers...>::watchdog() {
            if (expiration_ <= SteadyDuration::zero())
                co_return;

            try {
                do {
                    watchdog_.expires_at(received_ + expiration_);

                    if (auto [ec] = co_await watchdog_.async_wait(); ec) {
                        if (ec == asio::error::operation_aborted) {
                            // TODO
                        } else {
                            pipeline_.onError(ec);
                        }

                        co_return;
                    }

                    pipeline_.onTimeout();

                    if (isConnected()) {
                        disconnect();
                    }
                } while (received_ + expiration_ > std::chrono::steady_clock::now());
            } catch (const std::exception &e) {
                pipeline_.onException(e);
            }
        }
    }

    template<class Codec, class ...handlers>
    requires detail::kConnectionConcept<Codec, handlers...>
    shared_ptr<detail::ConnectionImpl<Codec, handlers...>> MakeConnection(TcpSocket &&socket) {
        return make_shared<detail::ConnectionImpl<Codec, handlers...>>(std::move(socket));
    }
}
