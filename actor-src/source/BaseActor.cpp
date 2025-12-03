#include "actor/BaseActor.h"
#include "actor/ActorContext.h"

namespace uranus::actor {
    BaseActor::BaseActor()
        : ctx_(nullptr) {
    }

    BaseActor::~BaseActor() {
    }

    ActorContext *BaseActor::getContext() const {
        return ctx_;
    }

    uint32_t BaseActor::getId() const {
        return ctx_->getId();
    }

    void BaseActor::setContext(ActorContext *ctx) {
        ctx_ = ctx;
    }
}
