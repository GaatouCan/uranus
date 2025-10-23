#pragma once

#include "Message.h"
#include "base/Types.h"

#include <memory>
#include <asio/ssl/stream.hpp>


namespace uranus {

    class MessageCodec;
    class GameServer;

    namespace network {
        class Package;
        class PackagePool;
    }
}

class Gateway;

using uranus::Message;
using uranus::MessageCodec;
using uranus::network::Package;
using uranus::network::PackagePool;
using uranus::GameServer;
using std::shared_ptr;
using std::unique_ptr;
using std::make_shared;
using std::make_unique;
using std::error_code;
using SslStream = asio::ssl::stream<TcpSocket>;


class Connection final : public std::enable_shared_from_this<Connection> {

public:
    Connection() = delete;

    Connection(SslStream &&stream, Gateway *gateway);
    ~Connection() ;

    DISABLE_COPY_MOVE(Connection)

    [[nodiscard]] std::string GetKey() const;

    [[nodiscard]] Gateway *GetGateway() const;
    [[nodiscard]] GameServer *GetGameServer() const;

    [[nodiscard]] asio::ip::address RemoteAddress() const;

    void SetExpiration(int sec);

    void ConnectToClient();
    void Disconnect();

    [[nodiscard]] bool IsConnected() const;

    void SetPlayerID(int64_t pid);
    [[nodiscard]] int64_t GetPlayerID() const;

    void SendToClient(const Message &msg);

private:
    awaitable<void> ReadPackage();
    awaitable<void> WritePackage();

    awaitable<void> Watchdog();

private:
    /// The Tcp socket with ssl
    SslStream stream_;

    /// The pointer to the gateway module
    Gateway *const gateway_;

    /// The unique key of this connection
    std::string key_;

    /// The codec for encoding/decoding the message
    unique_ptr<MessageCodec> codec_;

    /// The Object pool of the package
    shared_ptr<PackagePool> pool_;

    /// For output buffer
    unique_ptr<ConcurrentChannel<Message>> output_;

    /// Record the player id mapping the client
    int64_t pid_;

    /// Idle state timer
    SteadyTimer watchdog_;

    /// Watchdog expiration
    SteadyDuration expiration_;

    /// Will update after login and received the message
    SteadyTimePoint received_;
};
