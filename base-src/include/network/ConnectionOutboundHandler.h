#pragma once

#include "ConnectionHandler.h"
#include "ConnectionPipelineContext.h"

namespace uranus::network {

    class ConnectionOutboundHandler : virtual public ConnectionHandler {

    public:
        ConnectionOutboundHandler();
        ~ConnectionOutboundHandler() override;

        [[nodiscard]] Type type() const override;

        virtual awaitable<MessageHandle> beforeSend(ConnectionPipelineContext ctx, MessageHandle &&msg) = 0;
        virtual awaitable<void> afterSend(ConnectionPipelineContext ctx, MessageHandle &&msg) = 0;
    };
}
