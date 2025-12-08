#pragma once

#include "ConnectionHandler.h"
#include "ConnectionPipelineContext.h"

namespace uranus::network {

    // class ConnectionOutboundHandler : virtual public ConnectionHandler {
    //
    // public:
    //     ConnectionOutboundHandler();
    //     ~ConnectionOutboundHandler() override;
    //
    //     [[nodiscard]] Type type() const override;
    //
    //     virtual awaitable<MessageHandle> beforeSend(const ConnectionPipelineContext &ctx, MessageHandle &&msg);
    //     virtual awaitable<void> afterSend(const ConnectionPipelineContext &ctx, MessageHandle &&msg);
    // };
}
