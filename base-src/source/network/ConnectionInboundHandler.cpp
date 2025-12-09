#include "ConnectionInboundHandler.h"
#include "ConnectionPipelineContext.h"

namespace uranus::network {
    ConnectionInboundHandler::ConnectionInboundHandler() {
    }

    ConnectionInboundHandler::~ConnectionInboundHandler() {
    }

    ConnectionHandler::HandlerType ConnectionInboundHandler::type() const {
        return HandlerType::kInbound;
    }

    void ConnectionInboundHandler::onConnect(ConnectionPipelineContext &ctx) {
        ctx.fireConnect();
    }

    void ConnectionInboundHandler::onDisconnect(ConnectionPipelineContext &ctx) {
        ctx.fireDisconnect();
    }

    void ConnectionInboundHandler::onError(ConnectionPipelineContext &ctx, error_code ec) {
        ctx.fireError(ec);
    }

    void ConnectionInboundHandler::onException(ConnectionPipelineContext &ctx, exception &e) {
        ctx.fireException(e);
    }

    void ConnectionInboundHandler::onTimeout(ConnectionPipelineContext &ctx) {
        ctx.fireTimeout();
    }
}
