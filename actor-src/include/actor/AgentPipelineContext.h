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

        void fireReceive(Package *pkg) const;
        void fireSendPackage(PackageHandle &&pkg) const;

    private:
        AgentPipeline &pipeline_;
        const size_t index_;
    };

}
