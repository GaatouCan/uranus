#pragma once

#include <network/Connection.h>
#include <actor/Package.h>

using uranus::network::Connection;
using uranus::network::ConnectionInboundHandler;
using uranus::network::ConnectionPipelineContext;
using uranus::actor::Package;
using uranus::actor::PackageHandle;

class Gateway;
class GameWorld;

class GatewayHandler final : public ConnectionInboundHandler<Package> {

public:
    GatewayHandler();
    ~GatewayHandler() override;

    void setGateway(Gateway *gateway);

    [[nodiscard]] Gateway *getGateway() const;
    [[nodiscard]] GameWorld *getWorld() const;

    void onConnect(ConnectionPipelineContext &ctx) override;
    void onDisconnect(ConnectionPipelineContext &ctx) override;

    asio::awaitable<void> onReceive(ConnectionPipelineContext &ctx, PackageHandle &ref) override;

    void onError(ConnectionPipelineContext &ctx, std::error_code ec) override;
    void onException(ConnectionPipelineContext &ctx, const std::exception &e) override;
    void onTimeout(ConnectionPipelineContext &ctx) override;

private:
    Gateway *gateway_;
};
