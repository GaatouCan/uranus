#pragma once

#include "AbstractConnection.h"
#include "base/AttributeMap.h"
#include "base/types.h"

namespace uranus::network {

    using asio::awaitable;
    using std::error_code;

    class BASE_API BaseConnection : public AbstractConnection, public std::enable_shared_from_this<BaseConnection> {

    public:
        BaseConnection() = delete;

        explicit BaseConnection(TcpSocket &&socket);
        ~BaseConnection() override;

        DISABLE_COPY_MOVE(BaseConnection)

        [[nodiscard]] TcpSocket &socket();

        void connect() override;
        void disconnect() override;

        [[nodiscard]] bool isConnected() const override;

        [[nodiscard]] const std::string &getKey() const override;
        [[nodiscard]] asio::ip::address remoteAddress() const;

        AttributeMap &attr();

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
        ExecutorStrand strand_;

        std::string key_;
        AttributeMap attr_;

        SteadyTimer watchdog_;
        SteadyDuration expiration_;
        SteadyTimePoint received_;
    };
}
