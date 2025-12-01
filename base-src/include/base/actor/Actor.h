#pragma once

#include "base.export.h"
#include "Message.h"
#include "noncopy.h"

namespace uranus {
    class Message;
    struct Envelope;
}

namespace uranus::actor {

    class BASE_API Actor {

    public:
        Actor();
        virtual ~Actor();

        DISABLE_COPY_MOVE(Actor)

    protected:
        virtual void onMessage(const Message *msg) = 0;
    };
}
