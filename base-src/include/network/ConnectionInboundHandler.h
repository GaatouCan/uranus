#pragma once

#include "ConnectionHandler.h"
#include "Message.h"


namespace uranus::network {

    class ConnectionPipelineContext;

    class BASE_API ConnectionInboundHandler : virtual public ConnectionHandler {

    public:
        ConnectionInboundHandler();
        ~ConnectionInboundHandler() override;

        [[nodiscard]] Type type() const override;

        virtual void onReceive(ConnectionPipelineContext ctx, MessageHandle &&msg) = 0;
    };
}