#pragma once

#include "AgentHandler.h"

namespace uranus::actor {

    class ACTOR_API AgentOutboundHandler : virtual public AgentHandler {

    public:
        AgentOutboundHandler();
        ~AgentOutboundHandler() override;

        [[nodiscard]] HandlerType type() const override;
    };
}