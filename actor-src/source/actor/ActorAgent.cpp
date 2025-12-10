#include "ActorAgent.h"


namespace uranus::actor {
    ActorAgent::ActorAgent(asio::io_context &ctx)
        : ctx_(ctx),
          mailbox_(ctx_, 1024),
          id_(0) {
    }

    ActorAgent::~ActorAgent() {
    }

    void ActorAgent::setActor(ActorHandle &&handle) {
        if (actor_ != nullptr)
            return;

        if (!mailbox_.is_open())
            return;

        actor_ = std::move(handle);
        actor_->setContext(this);
    }

    BaseActor *ActorAgent::getActor() const {
        return actor_.get();
    }

    void ActorAgent::setId(uint32_t id) {
        id_ = id;
    }

    uint32_t ActorAgent::getId() const {
        return id_;
    }

    asio::io_context &ActorAgent::getIOContext() const {
        return ctx_;
    }

    bool ActorAgent::isRunning() const {
        return actor_ != nullptr && mailbox_.is_open();
    }

    AttributeMap &ActorAgent::attr() {
        return attr_;
    }

    void ActorAgent::pushEnvelope(Envelope &&envelope) {
        if (!mailbox_.is_open())
            return;

        if (actor_ == nullptr)
            return;

        mailbox_.try_send_via_dispatch(error_code{}, std::move(envelope));
    }
}
