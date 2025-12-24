#include "ActorContext.h"

#include <ranges>

#include "BaseActor.h"

#include <asio/detached.hpp>


namespace uranus::actor {
    ActorContext::SessionNode::SessionNode(asio::any_completion_handler<void(PackageHandle)> h, uint32_t s)
        : handle(std::move(h)),
          work(asio::make_work_guard(handle)),
          sess(s){
    }

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

                if ((envelope.type & Package::kRequest) != 0) {

                } else if ((envelope.type & Package::kResponse) != 0) {
                    SessionNode *node = nullptr;

                    // Find the node
                    {
                        unique_lock lock(sessMutex_);
                        if (const auto it = sessions_.find(envelope.session); it != sessions_.end()) {
                            node = it->second;
                            sessions_.erase(it);
                        }
                    }

                    if (node != nullptr) {
                        const auto work = asio::make_work_guard(node->handle);
                        const auto alloc = asio::get_associated_allocator(node->handle, asio::recycling_allocator<void>());
                        asio::dispatch(
                            work.get_executor(),
                            asio::bind_allocator(
                                alloc,
                                [handler = std::move(node->handle), res = std::move(envelope.package)]() mutable {
                                    std::move(handler)(std::move(res));
                                }
                            )
                        );

                        delete node;
                    }
                } else {
                    handle_->onPackage(std::move(envelope));
                }
            }

            // Release all sessions
            {
                for (const auto &node: sessions_ | std::views::values) {
                    if (!node)
                        continue;

                    auto alloc = asio::get_associated_allocator(node->handle, asio::recycling_allocator<void>());

                    asio::dispatch(
                        node->work.get_executor(),
                        asio::bind_allocator(
                            alloc,
                            [handler = std::move(node->handle)]() mutable {
                                std::move(handler)(nullptr);
                            }));

                    delete node;
                }

                sessions_.clear();
            }

            // Call actor terminate
            handle_->onTerminate();
        } catch (std::exception &e) {
            onException(e);
        }
    }

    int64_t ActorContext::pushSession(SessionHandle &&handle) {
        if (!isRunning()) {
            const auto work = asio::make_work_guard(handle);
            const auto alloc = asio::get_associated_allocator(handle, asio::recycling_allocator<void>());
            asio::dispatch(
                work.get_executor(),
                asio::bind_allocator(
                    alloc,
                    [handler = std::move(handle)]() mutable {
                        std::move(handler)(nullptr);
                    }
                )
            );

            return -1;
        }

        const auto sess = sessIdAlloc_.allocate();

        unique_lock lock(sessMutex_);
        if (sessions_.contains(sess)) {
            lock.unlock();

            const auto work = asio::make_work_guard(handle);
            const auto alloc = asio::get_associated_allocator(handle, asio::recycling_allocator<void>());
            asio::dispatch(
                work.get_executor(),
                asio::bind_allocator(
                    alloc,
                    [handler = std::move(handle)]() mutable {
                        std::move(handler)(nullptr);
                    }
                )
            );

            return -2;
        }

        auto *node = new SessionNode(std::move(handle), sess);
        sessions_.insert_or_assign(sess, node);

        return sess;
    }
}
