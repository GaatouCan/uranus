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

        void fireInitial();
        void fireTerminate();

        void fireReceive(Package *pkg);
        void fireSendPackage(PackageHandle &&pkg);

    private:
        AgentPipeline &pipeline_;
        const size_t index_;
    };

}
