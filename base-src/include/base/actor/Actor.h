#pragma once

#include "base/base.export.h"
#include "base/Message.h"
#include "base/noncopy.h"

#include <memory>
#include <functional>

namespace uranus {
    class Message;
    struct Envelope;
}

namespace uranus::actor {

    using std::unique_ptr;
    using std::make_unique;

    class BASE_API Actor {

    public:
        Actor();
        virtual ~Actor();

        DISABLE_COPY_MOVE(Actor)

    protected:
        virtual void onMessage(Envelope &&envelope) = 0;
    };

    using ActorHandle = std::unique_ptr<Actor, std::function<void(Actor *)>>;
}
