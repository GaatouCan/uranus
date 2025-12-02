#pragma once

#include "base/Message.h"
#include "base/noncopy.h"



namespace uranus::actor {

    class ActorContext {

    public:
        ActorContext() = default;
        virtual ~ActorContext() = default;

        DISABLE_COPY_MOVE(ActorContext)

        virtual void sendMessage(MessageHandle &&msg) = 0;
        virtual void sendMessage(Message *msg) = 0;
    };

    template<typename T>
    class ActorContextImpl : public ActorContext {

    };
}