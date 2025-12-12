#pragma once

#include "base/AttributeMap.h"
#include "Message.h"
#include "base/types.h"
#include "base/noncopy.h"

#include <string>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <tuple>


namespace uranus::network {
    using asio::co_spawn;
    using asio::detached;
    using asio::awaitable;

    using std::tuple;
    using std::make_tuple;
    using std::error_code;
    using std::shared_ptr;
    using std::unique_ptr;
    using std::make_shared;
    using std::make_unique;
    using std::enable_shared_from_this;

    class BASE_API Connection : public enable_shared_from_this<Connection> {

    public:
        Connection() = delete;

        explicit Connection(TcpSocket &&socket);
        virtual ~Connection();

        DISABLE_COPY_MOVE(Connection)

        TcpSocket &getSocket();

        virtual void connect();
        virtual void disconnect();

        [[nodiscard]] bool isConnected() const;

        [[nodiscard]] const std::string &getKey() const;
        [[nodiscard]] asio::ip::address remoteAddress() const;

        void setExpirationSecond(int sec);

        AttributeMap &attr();

        virtual void sendMessage(MessageHandle &&msg) = 0;
        virtual void sendMessage(MessageHandle *msg) = 0;

    protected:
        virtual awaitable<void> readMessage() = 0;
        virtual awaitable<void> writeMessage() = 0;

    private:
        awaitable<void> watchdog();

    protected:
        TcpSocket socket_;

        std::string key_;
        AttributeMap attr_;

    private:
        SteadyTimer watchdog_;
        SteadyDuration expiration_;
        SteadyTimePoint received_;
    };

    template<class T>
    requires std::is_base_of_v<Message, T>
    class MessageCodec {

    public:
        using MessageType = std::decay_t<T>;
        using MessageHandleType = Message::Pointer<MessageType>;

        MessageCodec() = delete;

        explicit MessageCodec(Connection &conn);
        virtual ~MessageCodec();

        DISABLE_COPY_MOVE(MessageCodec)

        [[nodiscard]] Connection &getConnection();
        [[nodiscard]] TcpSocket &getSocket();

        virtual awaitable<tuple<error_code, MessageHandleType>> decode() = 0;
        virtual awaitable<error_code> encode(MessageType *msg) = 0;

    private:
        Connection &conn_;
    };

    template<class T>
    requires std::is_base_of_v<Message, T>
    MessageCodec<T>::MessageCodec(Connection &conn)
        : conn_(conn) {
    }

    template<class T>
    requires std::is_base_of_v<Message, T>
    MessageCodec<T>::~MessageCodec() {
    }

    template<class T>
    requires std::is_base_of_v<Message, T>
    Connection &MessageCodec<T>::getConnection() {
        return conn_;
    }

    template<class T>
    requires std::is_base_of_v<Message, T>
    TcpSocket &MessageCodec<T>::getSocket() {
        return conn_.getSocket();
    }

    template<class Codec>
    concept kCodecType = requires { typename Codec::MessageType; }
        && std::derived_from<Codec, MessageCodec<typename Codec::MessageType>>;


    template<kCodecType Codec>
    class ConnectionImpl : public Connection {

    public:
        using MessageType = Codec::MessageType;
        using MessageHandleType = Codec::MessageHandleType;

        explicit ConnectionImpl(TcpSocket &&socket);
        ~ConnectionImpl() override;

        void connect() override;
        void disconnect() override;

        Codec &getCodec();

        void sendMessage(MessageHandle &&msg) override;
        void sendMessage(MessageHandle *msg) override;

    protected:
        awaitable<void> readMessage() override;
        awaitable<void> writeMessage() override;

        virtual void onReadMessage(MessageHandleType &&msg) = 0;
        virtual void beforeWrite(MessageType *msg) = 0;
        virtual void afterWrite(MessageHandleType &&msg) = 0;

        virtual void onTimeout() = 0;

    private:
        Codec codec_;
        ConcurrentChannel<MessageHandleType> output_;
    };

    template<kCodecType Codec>
    ConnectionImpl<Codec>::ConnectionImpl(TcpSocket &&socket)
        : Connection(std::move(socket)),
          output_(socket_.get_executor(), 1024) {
    }

    template<kCodecType Codec>
    ConnectionImpl<Codec>::~ConnectionImpl() {
    }

    template<kCodecType Codec>
    void ConnectionImpl<Codec>::connect() {
        Connection::connect();
    }

    template<kCodecType Codec>
    void ConnectionImpl<Codec>::disconnect() {
        Connection::disconnect();
        output_.cancel();
        output_.close();
    }

    template<kCodecType Codec>
    Codec &ConnectionImpl<Codec>::getCodec() {
        return codec_;
    }

    template<kCodecType Codec>
    void ConnectionImpl<Codec>::sendMessage(MessageHandle &&msg) {
    }

    template<kCodecType Codec>
    void ConnectionImpl<Codec>::sendMessage(MessageHandle *msg) {
    }

    template<kCodecType Codec>
    awaitable<void> ConnectionImpl<Codec>::readMessage() {
        try {
            while (isConnected()) {
                auto [ec, msg] = co_await codec_.decode();

                if (ec) {
                    // TODO
                    disconnect();
                    break;
                }

                this->onReadMessage(std::move(msg));
            }
        } catch (std::exception &e) {

        }
    }

    template<kCodecType Codec>
    awaitable<void> ConnectionImpl<Codec>::writeMessage() {
        try {
            while (isConnected() && output_.is_open()) {
                auto [ec, msg] = co_await output_.async_receive();

                if (ec == asio::error::operation_aborted ||
                    ec == asio::experimental::error::channel_closed) {
                    break;
                }

                if (ec) {
                    // TODO
                    disconnect();
                    break;
                }

                if (msg == nullptr)
                    continue;

                this->beforeWrite(msg.get());

                if (const auto writeEc = co_await codec_.encode(msg)) {
                    // TODO
                    disconnect();
                }

                this.afterWrite(std::move(msg));
            }
        } catch (std::exception &e) {

        }
    }
}
