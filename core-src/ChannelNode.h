#pragma once

#include "Common.h"

namespace uranus {

    class ActorContext;

    class CORE_API ChannelNode {

    public:
        ChannelNode() = default;
        virtual ~ChannelNode() = default;

        DISABLE_COPY_MOVE(ChannelNode)

        virtual void Execute(ActorContext *ctx) = 0;
    };
}