#include "ActorAgent.h"


namespace uranus::actor {
    ActorAgent::ActorAgent(asio::io_context &ctx)
        : ctx_(ctx),
          mailbox_(ctx_, 1024),
          pipeline_(*this),
          id_(0) {
    }

    ActorAgent::~ActorAgent() {
        terminate();
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

    void ActorAgent::run() {
        if (actor_ == nullptr) {
            // TODO
            return;
        }

        pipeline_.onInitial();

        co_spawn(ctx_, [self = shared_from_this()]() -> awaitable<void> {
            co_await self->process();
        }, detached);
    }

    void ActorAgent::terminate() {
        if (!mailbox_.is_open())
            return;

        mailbox_.cancel();
        mailbox_.close();
    }

    void ActorAgent::pushEnvelope(Envelope &&envelope) {
        if (!mailbox_.is_open())
            return;

        if (actor_ == nullptr)
            return;

        mailbox_.try_send_via_dispatch(error_code{}, std::move(envelope));
    }

    void ActorAgent::send(int32_t ty, uint32_t target, PackageHandle &&pkg) {
        if (pkg == nullptr)
            return;

        // pipeline_.onSendPackage(std::move(pkg));
    }

    void ActorAgent::send(const int32_t ty, const uint32_t target, Package *pkg) {
        send(ty, target, PackageHandle{ pkg, Message::Deleter::make() });
    }

    awaitable<void> ActorAgent::process() {
        try {
            while (mailbox_.is_open()) {
                auto [ec, envelope] = co_await mailbox_.async_receive();

                if (ec == asio::error::operation_aborted ||
                    ec == asio::experimental::error::channel_closed) {
                    // TODO
                    break;
                }

                if (ec) {
                    // TODO
                    continue;
                }

                pipeline_.onReceive(envelope.package.get());
                actor_->onMessage(std::move(envelope));
            }

            pipeline_.onTerminate();
        } catch (std::exception &e) {
            // TODO
        }
    }
}
