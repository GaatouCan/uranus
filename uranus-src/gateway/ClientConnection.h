#pragma once

#include <network/ConnectionAdapter.h>
#include <actor/PackageCodec.h>


namespace uranus {

    using network::ConnectionAdapter;
    using actor::Package;
    using actor::PackageHandle;
    using actor::PackageCodec;

    class Gateway;
    class GameWorld;

    class ClientConnection final : public ConnectionAdapter<PackageCodec> {

        friend class Gateway;

    public:
        explicit ClientConnection(TcpSocket &&socket);
        ~ClientConnection() override;

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
