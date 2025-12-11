#include "AgentOutboundHandler.h"

namespace uranus::actor {
    AgentOutboundHandler::AgentOutboundHandler() {
    }

    AgentOutboundHandler::~AgentOutboundHandler() {
    }

    AgentHandler::HandlerType AgentOutboundHandler::type() const {
        return HandlerType::kOutbound;
    }
}
