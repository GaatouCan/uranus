#include "actor/Actor.h"

namespace uranus::actor {
    Actor::Actor()
        : ctx_(nullptr) {
    }

    Actor::~Actor() {
    }

    ActorContext *Actor::getContext() const {
        return ctx_;
    }

    void Actor::setContext(ActorContext *ctx) {
        ctx_ = ctx;
    }
}
