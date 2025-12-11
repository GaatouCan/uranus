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
    }

    void AgentInboundHandler::onTerminate(AgentPipelineContext &ctx) {
    }

    void AgentInboundHandler::onReceive(AgentPipelineContext &ctx, Package *pkg) {
    }
}
