#include "BaseActor.h"
#include "ActorContext.h"

namespace uranus::actor {

    BaseActor::BaseActor()
        : ctx_(nullptr),
          enableTick_(false) {
    }

    BaseActor::~BaseActor() {
    }

    ActorContext *BaseActor::getContext() const {
        return ctx_;
    }

    void BaseActor::onInitial(ActorContext *ctx) {
        ctx_ = ctx;
    }

    void BaseActor::onStart(DataAsset *data) {
    }

    void BaseActor::onTerminate() {
    }

    void BaseActor::onTick(SteadyTimePoint now, SteadyDuration delta) {
    }

    ServerModule *BaseActor::getModule(const std::string &name) const {
        if (ctx_ == nullptr) {
            return nullptr;
        }
        return ctx_->getModule(name);
    }
}
