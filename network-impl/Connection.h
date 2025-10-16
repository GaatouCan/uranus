#pragma once

#include "base/Types.h"
#include "base/AttributeMap.h"

#include <memory>
#include <asio/ssl/stream.hpp>
#include <asio/experimental/concurrent_channel.hpp>

namespace uranus {

    class Message;

    namespace network {

        class Package;
        class PackagePool;
        class ConnectionHandler;

        using std::shared_ptr;
        using std::unique_ptr;
        using std::make_shared;
        using std::error_code;
        using SslStream = asio::ssl::stream<TcpSocket>;

        class Connection final : public AttributeMap, public std::enable_shared_from_this<Connection> {

            using OutputChannel = default_token::as_default_on_t<asio::experimental::concurrent_channel<void(error_code, unique_ptr<Message>)>>;

        public:
            Connection() = delete;

            explicit Connection(SslStream &&stream);
            ~Connection() override;

            // void SetGameServer(GameServer *server);
            // [[nodiscard]] GameServer *GetGameServer() const;

            void SetConnectionHandler(ConnectionHandler *handler);

            void ConnectToClient();
            void Disconnect();

            [[nodiscard]] bool IsConnected() const;

            void SendToClient(Message *msg);

        private:
            awaitable<void> ReadPackage();
            awaitable<void> WritePackage();

            awaitable<void> Watchdog();

        private:
            SslStream stream_;

            std::string key_;

            shared_ptr<PackagePool> pool_;

            OutputChannel output_;

            SteadyTimer watchdog_;
            SteadyDuration expiration_;
            SteadyTimePoint received_;

            unique_ptr<ConnectionHandler> handler_;
        };
    }
}
