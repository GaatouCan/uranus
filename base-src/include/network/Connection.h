#pragma once

#include "Message.h"
#include "noncopy.h"
#include "types.h"
#include "AttributeMap.h"

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
        enum class Type {
            kInbound,
            kOutbound,
            kDeluxe
        };

        ConnectionHandler() = default;
        virtual ~ConnectionHandler() = default;

        [[nodiscard]] virtual Type type() const = 0;
    };

    template<typename T>
    requires std::is_base_of_v<Message, T>
    class ConnectionInboundHandler : virtual public ConnectionHandler<T> {

    public:
        ConnectionInboundHandler() = default;
        ~ConnectionInboundHandler() override = default;

        [[nodiscard]] ConnectionHandler<T>::Type type() const override;
    };

    template<typename T>
    requires std::is_base_of_v<Message, T>
    class ConnectionOutboundHandler : virtual public ConnectionHandler<T> {

    public:
        ConnectionOutboundHandler() = default;
        ~ConnectionOutboundHandler() override = default;

        [[nodiscard]] ConnectionHandler<T>::Type type() const override;
    };

    template<typename T>
    requires std::is_base_of_v<Message, T>
    class ConnectionPipelineContext;

    template<typename T>
    requires std::is_base_of_v<Message, T>
    class ConnectionPipeline final {

        friend class ConnectionPipelineContext<T>;

    public:
        using MessageType = T;
        using MessageHandleType = Message::Pointer<MessageType>;
        using HandlerType = ConnectionHandler<MessageType>;

        ConnectionPipeline() = delete;

        explicit ConnectionPipeline(Connection &conn);
        ~ConnectionPipeline();

        DISABLE_COPY_MOVE(ConnectionPipeline)

        [[nodiscard]] Connection &getConnection() const;

    private:
        [[nodiscard]] tuple<size_t, ConnectionInboundHandler<T> *> getNextInboundHandler() const;
        [[nodiscard]] tuple<size_t, ConnectionOutboundHandler<T> *> getPreviousOutboundHandler() const;

    private:
        Connection &conn_;
        vector<unique_ptr<HandlerType>> handlers_;
    };

    template<typename T>
    requires std::is_base_of_v<Message, T>
    class ConnectionPipelineContext final {

    public:
        ConnectionPipelineContext() = delete;

        ConnectionPipelineContext(ConnectionPipeline<T> *pipeline, size_t idx);
        ~ConnectionPipelineContext();

        ConnectionPipelineContext(const ConnectionPipelineContext &rhs);
        ConnectionPipelineContext &operator=(const ConnectionPipelineContext &rhs);

        ConnectionPipelineContext(ConnectionPipelineContext &&rhs) noexcept;
        ConnectionPipelineContext &operator=(ConnectionPipelineContext &&rhs) noexcept;

        [[nodiscard]] Connection &getConnection() const;
        [[nodiscard]] ConnectionPipeline<T> &getPipeline() const;
        [[nodiscard]] AttributeMap &attr() const;

    private:
        ConnectionPipeline<T> *pipeline_;
        size_t index_;
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

    template<typename T>
    requires std::is_base_of_v<Message, T>
    ConnectionPipeline<T>::ConnectionPipeline(Connection &conn)
        : conn_(conn) {
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    ConnectionPipeline<T>::~ConnectionPipeline() = default;

    template<typename T>
    requires std::is_base_of_v<Message, T>
    Connection &ConnectionPipeline<T>::getConnection() const {
        return conn_;
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    tuple<size_t, ConnectionInboundHandler<T> *> ConnectionPipeline<T>::getNextInboundHandler() const {
        if (handlers_.empty())
            return make_tuple(0, nullptr);

        for (size_t idx = 0; idx < handlers_.size(); ++idx) {
            if (auto *temp = handlers_[idx].get(); temp->type() == ConnectionHandler<T>::Type::kInbound) {
                return make_tuple(idx, dynamic_cast<ConnectionInboundHandler<T> *>(temp));
            }
        }

        return make_tuple(0, nullptr);
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    tuple<size_t, ConnectionOutboundHandler<T> *> ConnectionPipeline<T>::getPreviousOutboundHandler() const {
        if (handlers_.empty())
            return make_tuple(0, nullptr);

        // Deal with while idx == 0
        for (size_t idx = handlers_.size(); idx-- > 0;) {
            if (auto *temp = handlers_[idx].get(); temp->type() == ConnectionHandler<T>::Type::kOutbound) {
                return make_tuple(idx, dynamic_cast<ConnectionOutboundHandler<T> *>(temp));
            }
        }

        return make_tuple(0, nullptr);
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    ConnectionPipelineContext<T>::ConnectionPipelineContext(ConnectionPipeline<T> *pipeline, const size_t idx)
        : pipeline_(pipeline),
          index_(idx){
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    ConnectionPipelineContext<T>::~ConnectionPipelineContext() = default;

    template<typename T>
    requires std::is_base_of_v<Message, T>
    ConnectionPipelineContext<T>::ConnectionPipelineContext(const ConnectionPipelineContext &rhs) {
        if (this != &rhs) {
            pipeline_ = rhs.pipeline_;
            index_ = rhs.index_;
        }
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    ConnectionPipelineContext<T> &ConnectionPipelineContext<T>::operator=(const ConnectionPipelineContext &rhs) {
        if (this != &rhs) {
            pipeline_ = rhs.pipeline_;
            index_ = rhs.index_;
        }
        return *this;
    }

    template<typename T> requires std::is_base_of_v<Message, T>
    ConnectionPipelineContext<T>::ConnectionPipelineContext(ConnectionPipelineContext &&rhs) noexcept {
        if (this != &rhs) {
            pipeline_ = rhs.pipeline_;
            index_ = rhs.index_;
            rhs.pipeline_ = nullptr;
            rhs.index_ = 0;
        }
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    ConnectionPipelineContext<T> &ConnectionPipelineContext<T>::operator=(ConnectionPipelineContext &&rhs) noexcept {
        if (this != &rhs) {
            pipeline_ = rhs.pipeline_;
            index_ = rhs.index_;
            rhs.pipeline_ = nullptr;
            rhs.index_ = 0;
        }
        return *this;
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    Connection &ConnectionPipelineContext<T>::getConnection() const {
        return pipeline_->getConnection();
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    ConnectionPipeline<T> &ConnectionPipelineContext<T>::getPipeline() const {
        return *pipeline_;
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    AttributeMap &ConnectionPipelineContext<T>::attr() const {
        return pipeline_->getConnection().attr();
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    ConnectionHandler<T>::Type ConnectionInboundHandler<T>::type() const {
        return ConnectionHandler<T>::Type::kInbound;
    }

    template<typename T>
    requires std::is_base_of_v<Message, T>
    ConnectionHandler<T>::Type ConnectionOutboundHandler<T>::type() const {
        return ConnectionHandler<T>::Type::kOutbound;
    }


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

                self->pipeline_->onConnect();

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

            pipeline_.onDisconnect();
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
                        pipeline_.onError(ec);
                        disconnect();
                        break;
                    }

                    received_ = std::chrono::steady_clock::now();
                    co_await pipeline_.onReceive(std::move(msg));
                }
            } catch (const std::exception &e) {
                pipeline_.onException(e);
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
                        pipeline_.onError(ec);
                        disconnect();
                        break;
                    }

                    if (msg == nullptr)
                        continue;

                    auto output = co_await pipeline_.beforeSend(std::move(msg));

                    if (const auto writeEc = co_await codec_.encode(output.get())) {
                        pipeline_.onError(writeEc);
                        disconnect();
                        break;
                    }

                    co_await pipeline_.afterSend(std::move(output));
                }
            } catch (const std::exception &e) {
                pipeline_.onException(e);
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
                            pipeline_.onError(ec);
                        }

                        co_return;
                    }

                    // handler_.onTimeout();

                    if (isConnected()) {
                        disconnect();
                    }
                } while (received_ + expiration_ > std::chrono::steady_clock::now());
            } catch (const std::exception &e) {
                pipeline_.onException(e);
            }
        }
    }

     template<class Codec, class Handler>
     requires std::is_base_of_v<MessageCodec<typename Codec::Type>, Codec>
     shared_ptr<detail::ConnectionImpl<Codec>> MakeConnection(TcpSocket &&socket) {
         return make_shared<detail::ConnectionImpl<Codec>>(std::move(socket));
     }
}
