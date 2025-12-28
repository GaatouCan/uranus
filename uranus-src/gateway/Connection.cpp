#include "Connection.h"
#include "Gateway.h"

#include "GameWorld.h"
#include "login/LoginAuth.h"
#include "player/PlayerManager.h"
#include "player/PlayerContext.h"

#include <spdlog/spdlog.h>


namespace uranus {

    using actor::Package;
    using actor::Envelope;

    Connection::Connection(ServerBootstrap &server, TcpSocket &&socket)
        : ConnectionAdapter(server, std::move(socket)),
          gateway_(nullptr) {
    }

    Connection::~Connection() {
    }

    Gateway *Connection::getGateway() const {
        return gateway_;
    }

    GameWorld *Connection::getWorld() const {
        if (gateway_)
            return &gateway_->getWorld();
        return nullptr;
    }

    void Connection::onConnect() {
    }

    void Connection::onDisconnect() {
        const auto op = attr().get<uint32_t>("PLAYER_ID");
        if (!op.has_value()) {
            return;
        }

        const auto pid = op.value();
        gateway_->onLogout(pid);

        if (auto *mgr = GetModule(PlayerManager)) {
            mgr->remove(pid);
        }
    }

    void Connection::onReadMessage(PackageHandle &&pkg) {
        if (!pkg)
            return;

        const auto op = attr().get<uint32_t>("PLAYER_ID");

        // Not login
        if (!op.has_value()) {
            if (auto *auth = GetModule(LoginAuth)) {
                auth->onPlayerLogin(pkg.get());
            }
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

        ctx->pushEnvelope(std::move(envelope));
    }

    void Connection::beforeWrite(Package *pkg) {
    }

    void Connection::afterWrite(PackageHandle &&pkg) {
    }

    void Connection::onTimeout() {
    }

    void Connection::onErrorCode(std::error_code ec) {
        SPDLOG_ERROR(ec.message());
    }

    void Connection::onException(std::exception &e) {
        SPDLOG_ERROR(e.what());
    }

    void Connection::setGateway(Gateway *gateway) {
        gateway_ = gateway;
    }
}
