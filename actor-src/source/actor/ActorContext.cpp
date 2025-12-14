#include "ActorContext.h"
#include "Actor.h"

namespace uranus::actor {
    ActorContext::ActorContext(asio::io_context &ctx)
        : ctx_(ctx),
          mailbox_(ctx_, 1024),
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

    Actor *ActorContext::getActor() const {
        return handle_.get();
    }

    bool ActorContext::isRunning() const {
        return mailbox_.is_open();
    }

    void ActorContext::pushEnvelope(Envelope &&envelope) {
        if (!isRunning())
            return;

        mailbox_.try_send_via_dispatch(std::error_code{}, std::move(envelope));
    }

    awaitable<void> ActorContext::process() {
        try {
            while (isRunning()) {
                auto [ec, envelope] = co_await mailbox_.async_receive();

            }
        } catch (std::exception &e) {

        }
    }
}
