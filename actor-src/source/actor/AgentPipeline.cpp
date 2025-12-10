#include "AgentPipeline.h"
#include "AgentHandler.h"

namespace uranus::actor {
    AgentPipeline::AgentPipeline(ActorAgent &agent)
        : agent_(agent) {
    }

    AgentPipeline::~AgentPipeline() {
    }

    ActorAgent &AgentPipeline::getAgent() const {
        return agent_;
    }
}
