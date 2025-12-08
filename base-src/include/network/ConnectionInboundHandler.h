#pragma once

#include "ConnectionHandler.h"
#include "ConnectionPipelineContext.h"

namespace uranus::network {

    // class BASE_API ConnectionInboundHandler : virtual public ConnectionHandler {
    //
    // public:
    //     ConnectionInboundHandler();
    //     ~ConnectionInboundHandler() override;
    //
    //     [[nodiscard]] Type type() const override;
    //
    //     virtual void onConnect(const ConnectionPipelineContext &ctx);
    //     virtual void onDisconnect(const ConnectionPipelineContext &ctx);
    //
    //     virtual awaitable<void> onReceive(const ConnectionPipelineContext &ctx, MessageHandle &&msg) = 0;
    //
    //     virtual void onError(const ConnectionPipelineContext &ctx, std::error_code ec);
    //     virtual void onException(const ConnectionPipelineContext &ctx, const std::exception &e);
    // };
}