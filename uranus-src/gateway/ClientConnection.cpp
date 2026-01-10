#include "ClientConnection.h"
#include "Gateway.h"
#include "GameWorld.h"
#include "common.h"

// #include "login/LoginAuth.h"
#include "player/PlayerManager.h"
#include "player/PlayerContext.h"

#include <spdlog/spdlog.h>


namespace uranus {

    using actor::Package;
    using actor::Envelope;

    ClientConnection::ClientConnection(TcpSocket &&socket)
        : ConnectionAdapter(std::move(socket)),
          gateway_(nullptr) {
    }

    ClientConnection::~ClientConnection() {
    }

    Gateway *ClientConnection::getGateway() const {
        return gateway_;
    }

    GameWorld *ClientConnection::getWorld() const {
        if (gateway_)
            return &gateway_->getWorld();
        return nullptr;
    }

    void ClientConnection::onConnect() {
    }

    void ClientConnection::onDisconnect() {
        const auto op = attr().get<uint32_t>("PLAYER_ID");
        if (!op.has_value()) {
            return;
        }

        const auto pid = op.value();
        gateway_->onLogout(pid);
    }

    void ClientConnection::onReadMessage(PackageHandle &&pkg) {
        if (!pkg)
            return;

        const auto op = attr().get<uint32_t>("PLAYER_ID");

        // Not login
        if (!op.has_value()) {
            // if (auto *auth = GetModule(LoginAuth)) {
            //     auth->onPlayerLogin(pkg.get(), key_);
            // }
            return;
        }

        const auto pid = op.value();

        const auto *mgr = GetModule(PlayerManager);
        if (!mgr)
            return;

        const auto ctx = mgr->find(pid);
        if (!ctx)
            return;

        Envelope envelope;

        envelope.type = (Package::kFromClient | Package::kToPlayer);
        envelope.source = pid;
        envelope.package = std::move(pkg);

        // ctx->pushEnvelope(std::move(envelope));
    }

    void ClientConnection::beforeWrite(Package *pkg) {
    }

    void ClientConnection::afterWrite(PackageHandle &&pkg) {
        if (pkg == nullptr)
            return;

        // Fail to login
        if (pkg->getId() == protocol::kLoginFailed ||
            pkg->getId() == protocol::kLoginRepeated) {
            attr().erase("PLAYER_ID");
            disconnect();
            return;
        }
    }

    void ClientConnection::onTimeout() {
    }

    void ClientConnection::onErrorCode(std::error_code ec) {
        SPDLOG_ERROR(ec.message());
    }

    void ClientConnection::onException(std::exception &e) {
        SPDLOG_ERROR(e.what());
    }

    void ClientConnection::setGateway(Gateway *gateway) {
        gateway_ = gateway;
    }
}
