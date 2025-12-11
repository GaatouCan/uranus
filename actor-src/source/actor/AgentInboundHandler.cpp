#include "AgentInboundHandler.h"
#include "AgentPipelineContext.h"

namespace uranus::actor {
    AgentInboundHandler::AgentInboundHandler() {
    }

    AgentInboundHandler::~AgentInboundHandler() {
    }

    AgentHandler::HandlerType AgentInboundHandler::type() const {
        return HandlerType::kInbound;
    }

    void AgentInboundHandler::onInitial(AgentPipelineContext &ctx) {
        ctx.fireInitial();
    }

    void AgentInboundHandler::onTerminate(AgentPipelineContext &ctx) {
        ctx.fireTerminate();
    }

    void AgentInboundHandler::onReceive(
        AgentPipelineContext &ctx,
        const int32_t ty,
        const uint32_t src,
        Package *pkg) {
        ctx.fireReceive(ty, src, pkg);
    }
}
