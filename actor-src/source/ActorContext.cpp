#include "ActorContext.h"


namespace uranus::actor {
    ActorContext::ActorContext(asio::io_context &ctx)
        : ctx_(ctx),
          mailbox_(ctx_, 1024),
          id_(0) {
    }

    ActorContext::~ActorContext() {
    }

    void ActorContext::setActor(ActorHandle &&handle) {
        if (actor_ != nullptr)
            return;

        if (!mailbox_.is_open())
            return;

        actor_ = std::move(handle);
        actor_->setContext(this);
    }

    BaseActor *ActorContext::getActor() const {
        return actor_.get();
    }

    void ActorContext::setId(uint32_t id) {
        id_ = id;
    }

    uint32_t ActorContext::getId() const {
        return id_;
    }

    asio::io_context &ActorContext::getIOContext() const {
        return ctx_;
    }

    bool ActorContext::isRunning() const {
        return actor_ != nullptr && mailbox_.is_open();
    }

    AttributeMap &ActorContext::attr() {
        return attr_;
    }

    void ActorContext::pushEnvelope(Envelope &&envelope) {
        if (!mailbox_.is_open())
            return;

        if (actor_ == nullptr)
            return;

        mailbox_.try_send_via_dispatch(error_code{}, std::move(envelope));
    }
}
