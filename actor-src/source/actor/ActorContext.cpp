#include "ActorContext.h"
#include "Actor.h"

namespace uranus::actor {
    ActorContext::ActorContext(asio::io_context &ctx)
        : ctx_(ctx),
          id_(0) {
    }

    ActorContext::~ActorContext() {
    }

    void ActorContext::setId(const uint32_t id) {
        id_ = id;
    }

    uint32_t ActorContext::getId() const {
        return id_;
    }

    void ActorContext::setUpActor(ActorHandle &&handle) {
        if (!handle)
            return;

        if (handle_)
            return;

        handle_ = std::move(handle);
        handle->setContext(this);
    }
}
