#pragma once

#include <base/Connection.h>
#include <actor/Package.h>

using uranus::Connection;
using uranus::ConnectionHandler;
using uranus::actor::Package;
using uranus::actor::PackageHandle;

class Gateway;
class GameWorld;

class GatewayHandler final : public ConnectionHandler<Package> {

public:
    explicit GatewayHandler(Connection &conn);
    ~GatewayHandler() override;

    void setGateway(Gateway *gateway);

    [[nodiscard]] Gateway *getGateway() const;
    [[nodiscard]] GameWorld *getWorld() const;

    void onConnect() override;
    void onDisconnect() override;

    void onError(std::error_code ec) override;
    void onException(const std::exception &e) override;

    void onReceive(PackageHandle &&pkg) override;
    void onWrite(Package *pkg) override;

private:
    Gateway *gateway_;
};
