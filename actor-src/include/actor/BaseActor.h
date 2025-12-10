#pragma once

#include "actor.export.h"

#include <base/noncopy.h>
#include <memory>
#include <functional>


namespace uranus {
    class Message;
}

namespace uranus::actor {

    using std::unique_ptr;
    using std::make_unique;

    class ActorAgent;
    class Envelope;

    class ACTOR_API BaseActor {

        friend class ActorAgent;

    public:
        BaseActor();
        virtual ~BaseActor();

        DISABLE_COPY_MOVE(BaseActor)

        [[nodiscard]] ActorAgent *getContext() const;

        [[nodiscard]] uint32_t getId() const;

        virtual void onMessage(Envelope &&envelope) = 0;

    private:
        void setContext(ActorAgent *ctx);

    private:
        ActorAgent *ctx_;
    };

    using ActorHandle = std::unique_ptr<BaseActor, std::function<void(BaseActor *)>>;
}
