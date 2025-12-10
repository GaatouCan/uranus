#pragma once

#include "actor.export.h"

#include <base/noncopy.h>
#include <vector>
#include <memory>

namespace uranus::actor {

    using std::vector;
    using std::unique_ptr;
    using std::make_unique;

    class ActorAgent;
    class AgentPipelineContext;
    class AgentHandler;

    class ACTOR_API AgentPipeline final {

        friend class AgentPipelineContext;

    public:
        AgentPipeline() = delete;

        explicit AgentPipeline(ActorAgent &agent);
        ~AgentPipeline();

        DISABLE_COPY_MOVE(AgentPipeline)

        [[nodiscard]] ActorAgent &getAgent() const;

    private:
        ActorAgent &agent_;

        vector<unique_ptr<AgentHandler>> handlers_;
    };

}