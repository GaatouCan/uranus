#pragma once

#include "MessageCodec.h"

namespace uranus::network {
    template<class Codec>
    concept kCodecType = requires { typename Codec::MessageType; }
        && std::derived_from<Codec, MessageCodec<typename Codec::MessageType>>;

    template<kCodecType Codec>
    class ConnectionAdapter : public BaseConnection {

    public:
        using CodecType = Codec;
        using MessageType = Codec::MessageType;
        using MessageHandleType = Codec::MessageHandleType;

        explicit ConnectionAdapter(TcpSocket &&socket);
        ~ConnectionAdapter() override;

        void connect() override;
        void disconnect() override;

        Codec &codec();

        void sendMessage(MessageHandle &&msg) override;
        void sendMessage(Message *msg) override;

        void send(MessageHandleType &&msg);
        void send(MessageType *msg);

    protected:
        awaitable<void> readLoop() override;
        awaitable<void> writeLoop() override;

        virtual void onReadMessage(MessageHandleType &&msg) = 0;
        virtual void beforeWrite(MessageType *msg) = 0;
        virtual void afterWrite(MessageHandleType &&msg) = 0;

    private:
        Codec codec_;
        ConcurrentChannel<MessageHandleType> output_;
    };

    template<kCodecType Codec>
    ConnectionAdapter<Codec>::ConnectionAdapter(TcpSocket &&socket)
        : BaseConnection(std::move(socket)),
          codec_(dynamic_cast<BaseConnection &>(*this)),
          output_(socket_.get_executor(), 1024) {
    }

    template<kCodecType Codec>
    ConnectionAdapter<Codec>::~ConnectionAdapter() {
    }

    template<kCodecType Codec>
    void ConnectionAdapter<Codec>::connect() {
        BaseConnection::connect();
    }

    template<kCodecType Codec>
    void ConnectionAdapter<Codec>::disconnect() {
        if (!isConnected())
            return;

#ifdef URANUS_SSL
        socket_.next_layer().close();
#else
        socket_.close();
#endif

        watchdog_.cancel();

        output_.cancel();
        output_.close();

        onDisconnect();
    }

    template<kCodecType Codec>
    Codec &ConnectionAdapter<Codec>::codec() {
        return codec_;
    }

    template<kCodecType Codec>
    void ConnectionAdapter<Codec>::sendMessage(MessageHandle &&msg) {
        if (msg == nullptr)
            return;

        auto del = msg.get_deleter();
        auto *ptr = msg.release();

        if (auto *temp = dynamic_cast<MessageType *>(ptr)) {
            MessageHandleType handle{ temp, del };
            this->send(std::move(handle));
            return;
        }

        del(ptr);
    }

    template<kCodecType Codec>
    void ConnectionAdapter<Codec>::sendMessage(Message *msg) {
        if (msg == nullptr)
            return;

        if (auto *temp = dynamic_cast<MessageType *>(msg)) {
            this->send(temp);
            return;
        }

        delete msg;
    }

    template<kCodecType Codec>
    void ConnectionAdapter<Codec>::send(MessageHandleType &&msg) {
        if (msg == nullptr)
            return;

        if (isConnected() && output_.is_open()) {
            output_.try_send_via_dispatch(error_code{}, std::move(msg));
        }
    }

    template<kCodecType Codec>
    void ConnectionAdapter<Codec>::send(MessageType *msg) {
        if (msg == nullptr)
            return;

        MessageHandleType handle{ msg, Message::Deleter::make() };
        this->send(std::move(handle));
    }

    template<kCodecType Codec>
    awaitable<void> ConnectionAdapter<Codec>::readLoop() {
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
            onException(e);
            disconnect();
        }
    }

    template<kCodecType Codec>
    awaitable<void> ConnectionAdapter<Codec>::writeLoop() {
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
                    onErrorCode(writeEc);
                    disconnect();
                }

                this->afterWrite(std::move(msg));
            }
        } catch (std::exception &e) {
            onException(e);
            disconnect();
        }
    }
}
