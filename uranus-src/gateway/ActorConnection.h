#pragma once

#include <network/Connection.h>
#include <actor/PackageCodec.h>

using uranus::network::ConnectionImpl;
using uranus::actor::PackageCodec;
using uranus::network::ServerBootstrap;
using uranus::TcpSocket;


class ActorConnection : public ConnectionImpl<PackageCodec> {

public:
    ActorConnection(ServerBootstrap &server, TcpSocket &&socket);
    ~ActorConnection() override;
};
