#include "ActorConnection.h"
#include "Gateway.h"

#include <spdlog/spdlog.h>

ActorConnection::ActorConnection(TcpSocket &&socket, Gateway &gateway)
    : ConnectionImpl(std::move(socket)),
      gateway_(gateway) {
}

ActorConnection::~ActorConnection() {
}

void ActorConnection::disconnect() {
    ConnectionImpl::disconnect();

    if (!attr().has("REPEATED")) {
        gateway_.remove(key_);
    }
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
