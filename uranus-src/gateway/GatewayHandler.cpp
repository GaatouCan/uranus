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

void GatewayHandler::onConnect(ConnectionPipelineContext &ctx) {

}

void GatewayHandler::onDisconnect(ConnectionPipelineContext &ctx) {

}

awaitable<void> GatewayHandler::onReceive(ConnectionPipelineContext &ctx, MessageHandleType &ref) {
}

void GatewayHandler::onError(ConnectionPipelineContext &ctx, std::error_code ec) {

}

void GatewayHandler::onException(ConnectionPipelineContext &ctx, const std::exception &e) {

}

void GatewayHandler::onTimeout(ConnectionPipelineContext &ctx) {

}
