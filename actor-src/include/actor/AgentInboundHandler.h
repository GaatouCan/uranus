#pragma once

#include "AgentHandler.h"

namespace uranus::actor {

    class ACTOR_API AgentInboundHandler : virtual public AgentHandler {

    public:
        AgentInboundHandler();
        ~AgentInboundHandler() override;

        [[nodiscard]] HandlerType type() const override;
    };
}