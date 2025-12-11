#include "AgentPipelineContext.h"
#include "AgentPipeline.h"
#include "AgentInboundHandler.h"
#include "AgentOutboundHandler.h"

namespace uranus::actor {
    AgentPipelineContext::AgentPipelineContext(AgentPipeline &pipeline, const size_t idx)
        : pipeline_(pipeline),
          index_(idx) {
    }

    AgentPipelineContext::~AgentPipelineContext() {
    }

    AgentPipeline &AgentPipelineContext::pipeline() const {
        return pipeline_;
    }

    void AgentPipelineContext::fireInitial() const {
        if (index_ >= pipeline_.inbounds_.size())
            return;

        AgentPipelineContext ctx(pipeline_, index_ + 1);
        pipeline_.inbounds_[index_]->onInitial(ctx);
    }

    void AgentPipelineContext::fireTerminate() const {
        if (index_ >= pipeline_.inbounds_.size())
            return;

        AgentPipelineContext ctx(pipeline_, index_ + 1);
        pipeline_.inbounds_[index_]->onTerminate(ctx);
    }

    void AgentPipelineContext::fireReceive(const int32_t ty, const uint32_t src, Package *pkg) const {
        if (index_ >= pipeline_.inbounds_.size())
            return;

        AgentPipelineContext ctx(pipeline_, index_ + 1);
        pipeline_.inbounds_[index_]->onReceive(ctx, ty, src, pkg);
    }

    void AgentPipelineContext::firePost(const uint32_t target, Envelope &&envelope) const {
        if (index_ >= pipeline_.outbounds_.size())
            return;

        AgentPipelineContext ctx(pipeline_, index_ + 1);
        pipeline_.outbounds_[index_]->onPost(ctx, target, std::move(envelope));
    }
}
