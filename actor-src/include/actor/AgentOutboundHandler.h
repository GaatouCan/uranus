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

        virtual void onSendPackage(AgentPipelineContext &ctx, PackageHandle &&pkg) = 0;
    };
}