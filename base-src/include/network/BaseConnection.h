#pragma once

#include "base/AttributeMap.h"
#include "base/Message.h"
#include "base/noncopy.h"
#include "base/types.h"

namespace uranus::network
{
    using asio::awaitable;
    using std::error_code;

    class BASE_API BaseConnection : public std::enable_shared_from_this<BaseConnection>
    {

    public:
        BaseConnection() = delete;

        explicit BaseConnection(TcpSocket &&socket);
        virtual ~BaseConnection();

        DISABLE_COPY_MOVE(BaseConnection)

        [[nodiscard]] TcpSocket &socket();

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

        virtual void onConnect() = 0;
        virtual void onDisconnect() = 0;

        virtual void onTimeout() = 0;
        virtual void onErrorCode(error_code ec) = 0;
        virtual void onException(std::exception &e) = 0;

    private:
        awaitable<void> watchdog();

    private:
        TcpSocket socket_;

        std::string key_;
        AttributeMap attr_;

        SteadyTimer watchdog_;
        SteadyDuration expiration_;
        SteadyTimePoint received_;
    };
}