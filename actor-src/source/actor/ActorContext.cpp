#include "ActorContext.h"
#include "BaseActor.h"

#include <asio/detached.hpp>


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
        handle_->setContext(this);
    }

    BaseActor *ActorContext::getActor() const {
        return handle_.get();
    }

    void ActorContext::run() {
        if (!handle_)
            throw std::runtime_error("ActorContext::run - Actor is null");

        // Call actor initial
        handle_->onInitial();

        co_spawn(ctx_, [self = shared_from_this()]() -> awaitable<void> {
            co_await self->process();
        }, asio::detached);
    }

    void ActorContext::terminate() {
        if (!mailbox_.is_open())
            return;

        mailbox_.cancel();
        mailbox_.close();
    }

    AttributeMap &ActorContext::attr() {
        return attr_;
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

                if (ec == asio::error::operation_aborted ||
                    ec == asio::experimental::error::channel_closed) {
                    break;
                }

                if (ec) {
                    onErrorCode(ec);
                    break;
                }

                handle_->onPackage(std::move(envelope));
            }

            // Call actor terminate
            handle_->onTerminate();
        } catch (std::exception &e) {
            onException(e);
        }
    }
}
