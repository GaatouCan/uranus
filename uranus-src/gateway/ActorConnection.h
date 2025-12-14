#pragma once

#include <network/Connection.h>
#include <actor/PackageCodec.h>


using uranus::TcpSocket;
using uranus::network::ConnectionImpl;
using uranus::actor::PackageCodec;
using uranus::actor::Package;
using uranus::actor::PackageHandle;

class Gateway;

class ActorConnection final : public ConnectionImpl<PackageCodec> {

public:
    ActorConnection(TcpSocket &&socket, Gateway &gateway);
    ~ActorConnection() override;

    void disconnect() override;

protected:
    void onReadMessage(PackageHandle &&pkg) override;

    void beforeWrite(Package *pkg) override;
    void afterWrite(PackageHandle &&pkg) override;

    void onTimeout() override;

    void onErrorCode(std::error_code ec) override;
    void onException(std::exception &e) override;

private:
    Gateway &gateway_;
};
