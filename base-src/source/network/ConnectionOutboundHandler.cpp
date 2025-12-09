#include "ConnectionOutboundHandler.h"
#include "ConnectionPipelineContext.h"

namespace uranus::network {
    ConnectionOutboundHandler::ConnectionOutboundHandler() {
    }

    ConnectionOutboundHandler::~ConnectionOutboundHandler() {
    }

    ConnectionHandler::HandlerType ConnectionOutboundHandler::type() const {
        return HandlerType::kOutbound;
    }

    awaitable<void> ConnectionOutboundHandler::beforeSend(ConnectionPipelineContext &ctx, Message *msg) {
        co_await ctx.fireBeforeSend(msg);
    }

    awaitable<void> ConnectionOutboundHandler::afterSend(ConnectionPipelineContext &ctx, MessageHandle &&msg) {
        co_await ctx.fireAfterSend(std::move(msg));
    }
}
