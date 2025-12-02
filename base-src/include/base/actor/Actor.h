#pragma once

#include "base/base.export.h"
#include "base/Message.h"
#include "base/noncopy.h"

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
        virtual void onMessage(const Envelope &envelope) = 0;
    };
}
