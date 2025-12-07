#include "ConnectionOutboundHandler.h"

namespace uranus::network {
    ConnectionOutboundHandler::ConnectionOutboundHandler() = default;
    ConnectionOutboundHandler::~ConnectionOutboundHandler() = default;

    ConnectionHandler::Type ConnectionOutboundHandler::type() const {
        return Type::kOutbound;
    }

    awaitable<MessageHandle> ConnectionOutboundHandler::beforeSend(const ConnectionPipelineContext &ctx, MessageHandle &&msg) {
        auto output = co_await ctx.fireBeforeSend(std::move(msg));
        co_return std::move(output);
    }

    awaitable<void> ConnectionOutboundHandler::afterSend(const ConnectionPipelineContext &ctx, MessageHandle &&msg) {
        co_await ctx.fireAfterSend(std::move(msg));
    }
}
