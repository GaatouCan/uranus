#include "ActorConnection.h"
#include "Gateway.h"

#include "../GameWorld.h"
#include "../player/PlayerManager.h"
#include "../player/PlayerContext.h"

#include <spdlog/spdlog.h>


namespace uranus {

    using actor::Package;
    using actor::Envelope;

    ActorConnection::ActorConnection(ServerBootstrap &server, TcpSocket &&socket)
        : ConnectionImpl(server, std::move(socket)),
          gateway_(nullptr) {
    }

    ActorConnection::~ActorConnection() {
    }

    Gateway *ActorConnection::getGateway() const {
        return gateway_;
    }

    GameWorld *ActorConnection::getWorld() const {
        if (gateway_)
            return &gateway_->getWorld();
        return nullptr;
    }

    void ActorConnection::onConnect() {
    }

    void ActorConnection::onDisconnect() {
        const auto op = attr().get<uint32_t>("PLAYER_ID");
        if (!op.has_value()) {
            return;
        }

        const auto pid = op.value();
        gateway_->onLogout(pid);
    }

    void ActorConnection::onReadMessage(PackageHandle &&pkg) {
        if (!pkg)
            return;

        const auto op = attr().get<uint32_t>("PLAYER_ID");

        // Not login
        if (!op.has_value()) {
            // TODO
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

    void ActorConnection::beforeWrite(Package *pkg) {
    }

    void ActorConnection::afterWrite(PackageHandle &&pkg) {
    }

    void ActorConnection::onTimeout() {
    }

    void ActorConnection::onErrorCode(std::error_code ec) {
        SPDLOG_ERROR(ec.message());
    }

    void ActorConnection::onException(std::exception &e) {
        SPDLOG_ERROR(e.what());
    }

    void ActorConnection::setGateway(Gateway *gateway) {
        gateway_ = gateway;
    }
}
