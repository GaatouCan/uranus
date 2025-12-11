#pragma once

#include "actor.export.h"

namespace uranus::actor {

    class AgentPipeline;

    class ACTOR_API AgentPipelineContext final {

        friend class AgentPipeline;

        AgentPipelineContext(AgentPipeline &pipeline, size_t idx);

    public:
        AgentPipelineContext() = delete;
        ~AgentPipelineContext();

        [[nodiscard]] AgentPipeline &pipeline() const;

    private:
        AgentPipeline &pipeline_;
        const size_t index_;
    };

}
