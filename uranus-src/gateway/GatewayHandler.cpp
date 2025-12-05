#include "GatewayHandler.h"
#include "Gateway.h"


GatewayHandler::GatewayHandler(Connection &conn)
    : ConnectionHandler(conn),
      gateway_(nullptr) {
}

GatewayHandler::~GatewayHandler() {
}

void GatewayHandler::setGateway(Gateway *gateway) {
    gateway_ = gateway;
}

Gateway *GatewayHandler::getGateway() const {
    return gateway_;
}

GameWorld *GatewayHandler::getWorld() const {
    if (!gateway_)
        return nullptr;

    return &gateway_->getWorld();
}

void GatewayHandler::onConnect() {
}

void GatewayHandler::onDisconnect() {
    if (!conn_.attr().has("REPEATED")) {
        getGateway()->removeConnection(conn_.getKey());
    }
}

void GatewayHandler::onError(std::error_code ec) {
}

void GatewayHandler::onException(const std::exception &e) {
}

void GatewayHandler::onReceive(PackageHandle &&pkg) {
}

void GatewayHandler::onWrite(Package *pkg) {
}
