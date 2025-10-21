#pragma once

#include "base/Types.h"

#include <memory>
#include <asio/ssl/stream.hpp>
#include <asio/experimental/concurrent_channel.hpp>

#include "Gateway.h"

namespace uranus {

    struct Message;
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

    using OutputChannel = default_token::as_default_on_t<asio::experimental::concurrent_channel<void(error_code, Message *)>>;

public:
    Connection() = delete;

    Connection(SslStream &&stream, Gateway *gateway);
    ~Connection() ;

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

    void SendToClient(Message *msg);

private:
    awaitable<void> ReadPackage();
    awaitable<void> WritePackage();

    awaitable<void> Watchdog();

private:
    SslStream stream_;
    Gateway *const gateway_;

    std::string key_;

    unique_ptr<MessageCodec> codec_;
    shared_ptr<PackagePool> pool_;

    unique_ptr<OutputChannel> output_;

    int64_t pid_;

    SteadyTimer watchdog_;
    SteadyDuration expiration_;
    SteadyTimePoint received_;
};
