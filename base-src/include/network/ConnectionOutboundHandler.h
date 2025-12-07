#pragma once

#include "Message.h"
#include "ConnectionHandler.h"

namespace uranus::network {
    class ConnectionOutboundHandler : virtual public ConnectionHandler {

    public:
        ConnectionOutboundHandler();
        ~ConnectionOutboundHandler() override;

        [[nodiscard]] Type type() const override;
    };
}
