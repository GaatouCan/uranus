#pragma once

#include "ConnectionHandler.h"
#include "ConnectionPipelineContext.h"

namespace uranus::network {

    class BASE_API ConnectionInboundHandler : virtual public ConnectionHandler {

    public:
        ConnectionInboundHandler();
        ~ConnectionInboundHandler() override;

        [[nodiscard]] Type type() const override;

        virtual void onConnect(ConnectionPipelineContext ctx) = 0;
        virtual void onDisconnect(ConnectionPipelineContext ctx) = 0;

        virtual awaitable<void> onReceive(ConnectionPipelineContext ctx, MessageHandle &&msg) = 0;

        virtual void onError(ConnectionPipelineContext ctx, std::error_code ec) = 0;
        virtual void onException(ConnectionPipelineContext ctx, const std::exception &e) = 0;
    };
}