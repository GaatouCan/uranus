#pragma once

#include "base/types.h"
#include "base/Message.h"
#include "base/AttributeMap.h"
#include "base/MultiIOContextPool.h"

#include <string>
#include <tuple>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>


namespace uranus::network {

    using asio::co_spawn;
    using asio::detached;
    using asio::awaitable;

    using std::tuple;
    using std::unordered_map;
    using std::make_tuple;
    using std::error_code;
    using std::shared_ptr;
    using std::unique_ptr;
    using std::make_shared;
    using std::make_unique;
    using std::enable_shared_from_this;


#pragma region Base Connection
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
        virtual void sendMessage(Message *msg) = 0;

    protected:
        virtual awaitable<void> readMessage() = 0;
        virtual awaitable<void> writeMessage() = 0;

        virtual void onTimeout() = 0;
        virtual void onErrorCode(error_code ec) = 0;
        virtual void onException(std::exception &e) = 0;

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
#pragma endregion

#pragma region MessageCodec
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

        [[nodiscard]] Connection &getConnection() const;
        [[nodiscard]] TcpSocket &getSocket() const;

        virtual awaitable<tuple<error_code, MessageHandleType>> decode() = 0;
        virtual awaitable<error_code> encode(MessageType *msg) = 0;

    private:
        Connection &conn_;
    };
#pragma endregion


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

        Codec &codec();

        void sendMessage(MessageHandle &&msg) override;
        void sendMessage(Message *msg) override;

        void send(MessageHandleType &&msg);
        void send(MessageType *msg);

    protected:
        awaitable<void> readMessage() override;
        awaitable<void> writeMessage() override;

        virtual void onReadMessage(MessageHandleType &&msg) = 0;
        virtual void beforeWrite(MessageType *msg) = 0;
        virtual void afterWrite(MessageHandleType &&msg) = 0;

    private:
        Codec codec_;
        ConcurrentChannel<MessageHandleType> output_;
    };

    template<class T>
    requires std::is_base_of_v<Message, T>
    MessageCodec<T>::MessageCodec(Connection &conn)
        : conn_(conn) {
    }

    template<class T>
    requires std::is_base_of_v<Message, T>
    MessageCodec<T>::~MessageCodec() = default;

    template<class T>
    requires std::is_base_of_v<Message, T>
    Connection &MessageCodec<T>::getConnection() const {
        return conn_;
    }

    template<class T>
    requires std::is_base_of_v<Message, T>
    TcpSocket &MessageCodec<T>::getSocket() const {
        return conn_.getSocket();
    }


    template<kCodecType Codec>
    ConnectionImpl<Codec>::ConnectionImpl(TcpSocket &&socket)
        : Connection(std::move(socket)),
          codec_(dynamic_cast<Connection &>(*this)),
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
    Codec &ConnectionImpl<Codec>::codec() {
        return codec_;
    }

    template<kCodecType Codec>
    void ConnectionImpl<Codec>::sendMessage(MessageHandle &&msg) {
        if (msg == nullptr)
            return;

        auto del = msg.get_deleter();
        auto *ptr = msg.get();

        if (auto *temp = dynamic_cast<MessageType *>(ptr)) {
            MessageHandleType handle{ temp, del };
            this->send(std::move(handle));
            return;
        }

        del(ptr);
    }

    template<kCodecType Codec>
    void ConnectionImpl<Codec>::sendMessage(Message *msg) {
        if (msg == nullptr)
            return;

        if (auto *temp = dynamic_cast<MessageType *>(msg)) {
            this->send(temp);
            return;
        }

        delete msg;
    }

    template<kCodecType Codec>
    void ConnectionImpl<Codec>::send(MessageHandleType &&msg) {
        if (msg == nullptr)
            return;

        if (isConnected() && output_.is_open()) {
            output_.try_send_via_dispatch(error_code{}, std::move(msg));
        }
    }

    template<kCodecType Codec>
    void ConnectionImpl<Codec>::send(MessageType *msg) {
        if (msg == nullptr)
            return;

        MessageHandleType handle{msg, Message::Deleter::make()};
        this->send(std::move(handle));
    }

    template<kCodecType Codec>
    awaitable<void> ConnectionImpl<Codec>::readMessage() {
        try {
            while (isConnected()) {
                auto [ec, msg] = co_await codec_.decode();

                if (ec) {
                    this->onErrorCode(ec);
                    disconnect();
                    break;
                }

                this->onReadMessage(std::move(msg));
            }
        } catch (std::exception &e) {
            this->onException(e);
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
                    this->onErrorCode(ec);
                    disconnect();
                    break;
                }

                if (msg == nullptr)
                    continue;

                this->beforeWrite(msg.get());

                if (const auto writeEc = co_await codec_.encode(msg.get())) {
                    this->onErrorCode(writeEc);
                    disconnect();
                }

                this->afterWrite(std::move(msg));
            }
        } catch (std::exception &e) {
            this->onException(e);
        }
    }
}
