#include "BaseActor.h"
#include "ActorAgent.h"

namespace uranus::actor {
    BaseActor::BaseActor()
        : ctx_(nullptr) {
    }

    BaseActor::~BaseActor() {
    }

    ActorAgent *BaseActor::getContext() const {
        return ctx_;
    }

    uint32_t BaseActor::getId() const {
        return ctx_->getId();
    }

    void BaseActor::setContext(ActorAgent *ctx) {
        ctx_ = ctx;
    }
}
