#include "ActorContext.h"
#include "BaseActor.h"

#include <asio/detached.hpp>
#include <asio/bind_allocator.hpp>
#include <ranges>

namespace uranus::actor {
    ActorContext::SessionNode::SessionNode(SessionHandle &&h, uint32_t s)
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

    auto ActorContext::call(uint32_t target, PackageHandle &&pkg) -> awaitable<PackageHandle> {
        auto token = asio::use_awaitable;
        return asio::async_initiate<asio::use_awaitable_t<>, void(PackageHandle)>([this](
            asio::completion_handler_for<void(PackageHandle)> auto handler,
            const uint32_t dest,
            PackageHandle &&temp
        ) mutable {
            if (!isRunning()) {
                auto work = asio::make_work_guard(handler);
                const auto alloc = asio::get_associated_allocator(handler, asio::recycling_allocator<void>());
                asio::dispatch(
                    work.get_executor(),
                    asio::bind_allocator(alloc, [handler = std::move(handler)]() mutable {
                        std::move(handler)(nullptr);
                    })
                );
                return;
            }

            const auto sess = sessAlloc_.allocate();

            {
                unique_lock lock(sessMutex_);

                if (sessions_.contains(sess)) {
                    lock.unlock();
                    const auto work = asio::make_work_guard(handler);
                    const auto alloc = asio::get_associated_allocator(handler, asio::recycling_allocator<void>());
                    asio::dispatch(
                        work.get_executor(),
                        asio::bind_allocator(alloc, [handler = std::move(handler)]() mutable {
                            std::move(handler)(nullptr);
                        })
                    );
                    return;
                }

                auto *node = new SessionNode(std::move(handler), sess);
                sessions_[sess] = node;
            }

            const auto ret = sendRequest(sess, dest, std::move(temp));

            // Delete the node and dispatch the nullptr result while ::call failed.
            if (!ret) {
                const SessionNode *node = nullptr;

                {
                    unique_lock lock(sessMutex_);
                    if (const auto iter = sessions_.find(sess); iter != sessions_.end()) {
                       node = iter->second;
                       sessions_.erase(iter);
                   }
                }

                if (node != nullptr) {
                    const auto alloc = asio::get_associated_allocator(node->handle, asio::recycling_allocator<void>());
                    asio::dispatch(
                        node->work.get_executor(),
                        asio::bind_allocator(alloc, [handler = std::move(handler)]() mutable {
                                std::move(handler)(nullptr);
                        })
                    );

                    delete node;
                }
            }
        }, token, target, std::move(pkg));
    }

    void ActorContext::onErrorCode(std::error_code ec) {
    }

    void ActorContext::onException(std::exception &e) {
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
                    const auto sess = envelope.session;
                    const auto from = envelope.source;

                    auto res = handle_->onRequest(std::move(envelope));

                    sendResponse(sess, from, std::move(res));
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
                        asio::bind_allocator(alloc, [handler = std::move(node->handle)]() mutable {
                            std::move(handler)(nullptr);
                        })
                    );

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
}
