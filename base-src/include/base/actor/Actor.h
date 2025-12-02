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

    class ActorContext;

    class BASE_API Actor {

        friend class ActorContext;

    public:
        Actor();
        virtual ~Actor();

        DISABLE_COPY_MOVE(Actor)

        void setContext(ActorContext *ctx);
        [[nodiscard]] ActorContext *getContext() const;

    protected:
        virtual void onMessage(Envelope &&envelope) = 0;

    private:
        ActorContext *ctx_;
    };

    using ActorHandle = std::unique_ptr<Actor, std::function<void(Actor *)>>;
}
