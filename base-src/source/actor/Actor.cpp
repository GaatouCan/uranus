#include "actor/Actor.h"
#include "actor/ActorContext.h"

namespace uranus::actor {
    Actor::Actor()
        : ctx_(nullptr) {
    }

    Actor::~Actor() {
    }

    ActorContext *Actor::getContext() const {
        return ctx_;
    }

    uint32_t Actor::getId() const {
        return ctx_->getId();
    }

    void Actor::setContext(ActorContext *ctx) {
        ctx_ = ctx;
    }
}
