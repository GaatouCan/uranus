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
    class GameWorld;

    class ActorConnection final : public ConnectionImpl<PackageCodec> {

        friend class Gateway;

    public:
        ActorConnection(ServerBootstrap &Server, TcpSocket &&socket);
        ~ActorConnection() override;

        [[nodiscard]] Gateway *getGateway() const;
        [[nodiscard]] GameWorld *getWorld() const;

    protected:
        void onConnect() override;
        void onDisconnect() override;

        void onReadMessage(PackageHandle &&pkg) override;

        void beforeWrite(Package *pkg) override;
        void afterWrite(PackageHandle &&pkg) override;

        void onTimeout() override;

        void onErrorCode(std::error_code ec) override;
        void onException(std::exception &e) override;

    private:
        void setGateway(Gateway *gateway);

    private:
        Gateway *gateway_;
    };
}