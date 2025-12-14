#pragma once

#include <base/network.h>
#include <actor/PackageCodec.h>

namespace uranus {

    using network::ServerBootstrap;
    using network::ConnectionImpl;
    using actor::Package;
    using actor::PackageHandle;
    using actor::PackageCodec;

    class Gateway;

    class ActorConnection final : public ConnectionImpl<PackageCodec> {

    public:
        ActorConnection(ServerBootstrap &Server, TcpSocket &&socket);
        ~ActorConnection() override;

    protected:
        void onReadMessage(PackageHandle &&pkg) override;

        void beforeWrite(Package *pkg) override;
        void afterWrite(PackageHandle &&pkg) override;

        void onTimeout() override;

        void onErrorCode(std::error_code ec) override;
        void onException(std::exception &e) override;
    };
}