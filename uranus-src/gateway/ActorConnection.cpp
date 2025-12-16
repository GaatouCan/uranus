#include "ActorConnection.h"
#include "Gateway.h"

#include "../player/PlayerManager.h"
#include "../player/PlayerContext.h"

#include <spdlog/spdlog.h>

namespace uranus {
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
