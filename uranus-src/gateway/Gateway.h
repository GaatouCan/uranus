#pragma once

#include "GatewayHandler.h"

#include <base/ServerModule.h>
#include <base/MultiIOContextPool.h>
#include <actor/PackageCodec.h>
#include <unordered_map>


using uranus::ServerModule;
using uranus::ServerBootstrap;
using uranus::actor::PackageCodec;
using uranus::MultiIOContextPool;
using uranus::TcpAcceptor;
using uranus::TcpSocket;
using uranus::network::Connection;
using asio::awaitable;
using asio::co_spawn;
using asio::detached;

class GameWorld;

class Gateway final : public ServerModule {

    friend class GatewayHandler;

public:
    using ConnectionPointer = std::shared_ptr<uranus::network::detail::ConnectionImpl<PackageCodec, GatewayHandler>>;

    explicit Gateway(GameWorld &world);
    ~Gateway() override;

    constexpr const char *getModuleName() override {
        return "Gateway";
    }

    [[nodiscard]] GameWorld &getWorld() const;

    void start() override;
    void stop() override;

    [[nodiscard]] ConnectionPointer find(const std::string &key) const;

private:
    void removeConnection(const std::string &key);

private:
    awaitable<void> waitForClient(uint16_t port);

private:
    asio::io_context ctx_;
    asio::ssl::context sslContext_;
    TcpAcceptor acceptor_;

    MultiIOContextPool pool_;

    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, ConnectionPointer> connMap_;
};
