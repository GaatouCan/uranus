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
using uranus::Connection;
using asio::awaitable;
using asio::co_spawn;
using asio::detached;

class GameWorld;

class Gateway final : public ServerModule {

public:
    explicit Gateway(ServerBootstrap &server);
    ~Gateway() override;

    [[nodiscard]] GameWorld &getWorld() const;

    void start() override;
    void stop() override;

private:
    awaitable<void> waitForClient(uint16_t port);

private:
    asio::io_context ctx_;
    asio::ssl::context sslContext_;
    TcpAcceptor acceptor_;

    MultiIOContextPool pool_;

    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<Connection>> connMap_;
};
