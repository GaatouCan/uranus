#include "AgentPipelineContext.h"

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

    void AgentPipelineContext::fireInitial() {
    }

    void AgentPipelineContext::fireTerminate() {
    }

    void AgentPipelineContext::fireReceive(Package *pkg) {
    }

    void AgentPipelineContext::fireSendPackage(PackageHandle &&pkg) {
    }
}
