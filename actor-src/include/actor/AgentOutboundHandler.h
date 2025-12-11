#pragma once

#include "AgentHandler.h"
#include "Package.h"

namespace uranus::actor {

    class AgentPipelineContext;

    class ACTOR_API AgentOutboundHandler : virtual public AgentHandler {

    public:
        AgentOutboundHandler();
        ~AgentOutboundHandler() override;

        [[nodiscard]] HandlerType type() const override;

        virtual void onPost(AgentPipelineContext &ctx, uint32_t target, Envelope &&envelope) = 0;
    };
}