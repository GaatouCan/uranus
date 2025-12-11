#pragma once

#include "Package.h"


namespace uranus::actor {

    class AgentPipeline;

    class ACTOR_API AgentPipelineContext final {

        friend class AgentPipeline;

        AgentPipelineContext(AgentPipeline &pipeline, size_t idx);

    public:
        AgentPipelineContext() = delete;
        ~AgentPipelineContext();

        [[nodiscard]] AgentPipeline &pipeline() const;

        void fireInitial() const;
        void fireTerminate() const;

        void fireReceive(int32_t ty, uint32_t src, Package *pkg) const;
        void firePost(uint32_t target, Envelope &&envelope) const;

    private:
        AgentPipeline &pipeline_;
        const size_t index_;
    };

}
