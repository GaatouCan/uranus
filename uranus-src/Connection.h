#pragma once

#include "base/Types.h"

#include <memory>
#include <asio/ssl/stream.hpp>
#include <asio/experimental/concurrent_channel.hpp>

#include "Gateway.h"

namespace uranus {
    class Message;
    namespace network {
        class Package;
        class PackagePool;
    }
}

class Gateway;

using uranus::Message;
using uranus::network::Package;
using uranus::network::PackagePool;
using std::shared_ptr;
using std::unique_ptr;
using std::make_shared;
using std::error_code;
using SslStream = asio::ssl::stream<TcpSocket>;


class Connection final : public std::enable_shared_from_this<Connection> {

    using OutputChannel = default_token::as_default_on_t<asio::experimental::concurrent_channel<void(error_code, unique_ptr<Message>)>>;

public:
    Connection() = delete;

    Connection(SslStream &&stream, Gateway *gateway);
    ~Connection() ;

    [[nodiscard]] std::string GetKey() const;

    [[nodiscard]] Gateway *GetGateway() const;

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
    Gateway *const gateway_;

    std::string key_;
    int64_t pid_;

    shared_ptr<PackagePool> pool_;

    OutputChannel output_;

    SteadyTimer watchdog_;
    SteadyDuration expiration_;
    SteadyTimePoint received_;
};
