#include "BaseActor.h"

namespace uranus::actor {
    BaseActor::BaseActor()
        : ctx_(nullptr) {
    }

    BaseActor::~BaseActor() {
    }

    BaseActorContext *BaseActor::getContext() const {
        return ctx_;
    }

    void BaseActor::onInitial() {
    }

    void BaseActor::onTerminate() {
    }

    void BaseActor::setContext(BaseActorContext *ctx) {
        ctx_ = ctx;
    }
}
