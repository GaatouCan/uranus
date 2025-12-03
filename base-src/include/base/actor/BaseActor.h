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

    class BASE_API BaseActor {

        friend class ActorContext;

    public:
        BaseActor();
        virtual ~BaseActor();

        DISABLE_COPY_MOVE(BaseActor)

        void setContext(ActorContext *ctx);
        [[nodiscard]] ActorContext *getContext() const;

        [[nodiscard]] uint32_t getId() const;

    protected:
        virtual void onMessage(Envelope &&envelope) = 0;

    private:
        ActorContext *ctx_;
    };

    using ActorHandle = std::unique_ptr<BaseActor, std::function<void(BaseActor *)>>;
}
