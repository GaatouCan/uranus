#include "AgentPipeline.h"
#include "AgentPipelineContext.h"
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

    void AgentPipeline::onInitial() {
        if (!inbounds_.empty()) {
            AgentPipelineContext ctx(*this, 1);
            inbounds_.front()->onInitial(ctx);
        }
    }

    void AgentPipeline::onTerminate() {
        if (!inbounds_.empty()) {
            AgentPipelineContext ctx(*this, 1);
            inbounds_.front()->onTerminate(ctx);
        }
    }

    void AgentPipeline::onReceive(int32_t ty, uint32_t src, Package *pkg) {
        if (!inbounds_.empty()) {
            AgentPipelineContext ctx(*this, 1);
            inbounds_.front()->onReceive(ctx, ty, src, pkg);
        }
    }

    void AgentPipeline::onPost(const uint32_t target, Envelope &&envelope) {
        if (!outbounds_.empty()) {
            AgentPipelineContext ctx(*this, 1);
            outbounds_.front()->onPost(ctx, target, std::move(envelope));
        }
    }
}
