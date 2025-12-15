#include "ActorConnection.h"
#include "Gateway.h"

#include <spdlog/spdlog.h>

namespace uranus {

    ActorConnection::ActorConnection(ServerBootstrap &server, TcpSocket &&socket)
        : ConnectionImpl(server, std::move(socket)) {
    }

    ActorConnection::~ActorConnection() {
    }

    void ActorConnection::onConnect() {
    }

    void ActorConnection::onDisconnect() {
    }


    void ActorConnection::onReadMessage(PackageHandle &&pkg) {
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
}
