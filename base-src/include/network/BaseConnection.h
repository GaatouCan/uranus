#pragma once

#include "Connection.h"
#include "base/AttributeMap.h"
#include "base/types.h"

namespace uranus::network {

    using asio::awaitable;
    using std::error_code;
    using std::enable_shared_from_this;

    class BASE_API BaseConnection : public Connection, public enable_shared_from_this<BaseConnection> {

    public:
        BaseConnection() = delete;

        explicit BaseConnection(TcpSocket &&socket);
        ~BaseConnection() override;

        DISABLE_COPY_MOVE(BaseConnection)

        [[nodiscard]] TcpSocket &socket();

        void connect() override;
        void disconnect() override;

        [[nodiscard]] bool isConnected() const override;

        [[nodiscard]] asio::ip::address remoteAddress() const;

        AttributeMap &attr() override;
        [[nodiscard]] const AttributeMap &attr() const override;

        void setExpirationSecond(int sec);

    protected:
        virtual awaitable<void> readLoop()  = 0;
        virtual awaitable<void> writeLoop() = 0;

        virtual void onConnect()                    = 0;
        virtual void onDisconnect()                 = 0;
        virtual void onTimeout()                    = 0;
        virtual void onErrorCode(error_code ec)     = 0;
        virtual void onException(std::exception &e) = 0;

    private:
        awaitable<void> watchdog();

    protected:
        TcpSocket socket_;

        AttributeMap attr_;

        SteadyTimer watchdog_;
        SteadyDuration expiration_;
        SteadyTimePoint received_;
    };
}
