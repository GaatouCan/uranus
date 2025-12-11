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

    void AgentPipelineContext::fireReceive(Package *pkg) const {
        if (index_ >= pipeline_.inbounds_.size())
            return;

        AgentPipelineContext ctx(pipeline_, index_ + 1);
        pipeline_.inbounds_[index_]->onReceive(ctx, pkg);
    }

    void AgentPipelineContext::fireSendPackage(PackageHandle &&pkg) const {
        if (index_ >= pipeline_.outbounds_.size())
            return;

        AgentPipelineContext ctx(pipeline_, index_ + 1);
        pipeline_.outbounds_[index_]->onSendPackage(ctx, std::move(pkg));
    }
}
