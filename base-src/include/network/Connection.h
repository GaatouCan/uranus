#pragma once


#include "base/AttributeMap.h"
#include "MessageCodec.h"
#include "ConnectionPipeline.h"

#include <string>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>


namespace uranus::network {

    using asio::co_spawn;
    using asio::detached;

    using std::shared_ptr;
    using std::unique_ptr;
    using std::make_shared;
    using std::make_unique;
    using std::enable_shared_from_this;

    class BASE_API Connection final : public enable_shared_from_this<Connection> {

    public:
        Connection() = delete;

        explicit Connection(TcpSocket &&socket);
        ~Connection();

        DISABLE_COPY_MOVE(Connection)

        TcpSocket &getSocket();

        template<class T, class ... Args>
        requires std::is_base_of_v<MessageCodec, T>
        void setMessageCodec(Args &&... args);

        MessageCodec &codec() const;

        template<class T>
        requires std::is_base_of_v<MessageCodec, T>
        T *getCodecT() const noexcept;

        ConnectionPipeline &getPipeline();

        void connect();
        void disconnect();

        [[nodiscard]] bool isConnected() const;

        [[nodiscard]] const std::string &getKey() const;
        [[nodiscard]] asio::ip::address remoteAddress() const;

        void setExpirationSecond(int sec);

        void send(MessageHandle &&msg);
        void send(Message *msg);

        AttributeMap &attr();

    private:
        awaitable<void> readMessage();
        awaitable<void> writeMessage();
        awaitable<void> watchdog();

    protected:
        TcpSocket socket_;

        unique_ptr<MessageCodec> codec_;
        ConcurrentChannel<MessageHandle> output_;

        std::string key_;
        AttributeMap attr_;

        ConnectionPipeline pipeline_;

        SteadyTimer watchdog_;
        SteadyDuration expiration_;
        SteadyTimePoint received_;
    };

    template<class T, class ... Args>
    requires std::is_base_of_v<MessageCodec, T>
    void Connection::setMessageCodec(Args &&...args) {
        if (codec_ != nullptr)
            return;

        codec_ = make_unique<T>(*this, std::forward<Args>(args)...);
    }

    template<class T>
    requires std::is_base_of_v<MessageCodec, T>
    T *Connection::getCodecT() const noexcept {
        if (!codec_)
            return nullptr;

        return dynamic_cast<T *>(codec_.get());
    }

}
