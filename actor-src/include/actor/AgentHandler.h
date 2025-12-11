#pragma once

#include "actor.export.h"

namespace uranus::actor {

    class ACTOR_API AgentHandler {

    public:
        enum class HandlerType {
            kInbound,
            kOutbound,
            kDeluxe
        };

        AgentHandler();
        virtual ~AgentHandler();

        [[nodiscard]] virtual HandlerType type() const = 0;
    };
}