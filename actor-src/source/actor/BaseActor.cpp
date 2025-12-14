#include "BaseActor.h"

namespace uranus::actor {
    BaseActor::BaseActor()
        : ctx_(nullptr) {
    }

    BaseActor::~BaseActor() {
    }

    ActorContext *BaseActor::getContext() const {
        return ctx_;
    }

    void BaseActor::onInitial() {
    }

    void BaseActor::onTerminate() {
    }

    void BaseActor::setContext(ActorContext *ctx) {
        ctx_ = ctx;
    }
}
