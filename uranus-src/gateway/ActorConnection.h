#pragma once

#include <network/Connection.h>
#include <actor/PackageCodec.h>

using uranus::TcpSocket;
using uranus::network::ConnectionImpl;
using uranus::network::ServerBootstrap;
using uranus::actor::PackageCodec;
using uranus::actor::Package;
using uranus::actor::PackageHandle;


class ActorConnection final : public ConnectionImpl<PackageCodec> {

public:
    ActorConnection(ServerBootstrap &server, TcpSocket &&socket);
    ~ActorConnection() override;

protected:
    void onReadMessage(PackageHandle &&pkg) override;

    void beforeWrite(Package *pkg) override;
    void afterWrite(PackageHandle &&pkg) override;

    void onTimeout() override;
};
