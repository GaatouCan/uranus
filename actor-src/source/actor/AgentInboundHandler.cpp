#include "AgentInboundHandler.h"

namespace uranus::actor {
    AgentInboundHandler::AgentInboundHandler() {
    }

    AgentInboundHandler::~AgentInboundHandler() {
    }

    AgentHandler::HandlerType AgentInboundHandler::type() const {
        return HandlerType::kInbound;
    }
}
