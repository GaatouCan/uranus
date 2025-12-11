#pragma once

#include "AgentHandler.h"
#include "Package.h"

namespace uranus::actor {

    class AgentPipelineContext;

    class ACTOR_API AgentInboundHandler : virtual public AgentHandler {

    public:
        AgentInboundHandler();
        ~AgentInboundHandler() override;

        [[nodiscard]] HandlerType type() const override;

        virtual void onInitial(AgentPipelineContext &ctx);
        virtual void onTerminate(AgentPipelineContext &ctx);

        virtual void onReceive(AgentPipelineContext &ctx, int32_t ty, uint32_t src, Package *pkg);
    };
}