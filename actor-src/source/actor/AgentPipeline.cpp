#include "AgentPipeline.h"
#include "AgentInboundHandler.h"
#include "AgentOutboundHandler.h"

namespace uranus::actor {
    AgentPipeline::AgentPipeline(ActorAgent &agent)
        : agent_(agent) {
    }

    AgentPipeline::~AgentPipeline() {
    }

    ActorAgent &AgentPipeline::getAgent() const {
        return agent_;
    }

    AgentPipeline &AgentPipeline::pushBack(AgentHandler *handler) {
        if (handler == nullptr)
            return *this;

        handlers_.emplace_back(handler);

        if (handler->type() == AgentHandler::HandlerType::kInbound ||
            handler->type() == AgentHandler::HandlerType::kDeluxe) {
            inbounds_.emplace_back(dynamic_cast<AgentInboundHandler *>(handler));
        }

        if (handler->type() == AgentHandler::HandlerType::kOutbound ||
            handler->type() == AgentHandler::HandlerType::kDeluxe) {
            outbounds_.insert(outbounds_.begin(), dynamic_cast<AgentOutboundHandler *>(handler));
        }

        return *this;
    }
}
