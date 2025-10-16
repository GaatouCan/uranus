#pragma once

#include "base/Types.h"

#include <memory>
#include <asio/ssl/stream.hpp>

namespace uranus {

    class GameServer;

    namespace network {

        using SslStream = asio::ssl::stream<TcpSocket>;

        class Connection final : public std::enable_shared_from_this<Connection> {

        public:
            Connection() = delete;

            explicit Connection(SslStream &&stream);
            ~Connection();

            void ConnectToClient();
            void Disconnect();

        private:
            awaitable<void> ReadPackage();
            awaitable<void> WritePackage();

            awaitable<void> Watchdog();

        private:
            SslStream stream_;
            GameServer *server_;
        };
    }
}
