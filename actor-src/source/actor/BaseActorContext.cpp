#include "BaseActorContext.h"
#include "BaseActor.h"

#include <asio/experimental/awaitable_operators.hpp>
#include <asio/detached.hpp>
#include <asio/bind_allocator.hpp>
#include <ranges>


namespace uranus::actor {

    using namespace asio::experimental::awaitable_operators;
    using asio::detached;

    BaseActorContext::SessionNode::SessionNode(asio::io_context &ctx, SessionHandler &&h, const int64_t s)
        : handler_(std::move(h)),
          guard_(asio::make_work_guard(handler_)),
          timer_(ctx),
          sess_(s) {

        // 发起超时等待
        co_spawn(ctx, [self = shared_from_this()]() mutable -> awaitable<void> {
            try {
                self->timer_.expires_after(std::chrono::seconds(5));

                // 即使返回error_code也需要唤醒协程 所以直接无论如何都唤醒
                co_await self->timer_.async_wait();

                // 超时返回空指针
                if (!self->completed_.test_and_set(std::memory_order_acquire)) {
                    const auto work = asio::make_work_guard(self->handler_);
                    const auto alloc = asio::get_associated_allocator(self->handler_, asio::recycling_allocator<void>());
                    asio::dispatch(
                        work.get_executor(),
                        asio::bind_allocator(alloc, [handler = std::move(self->handler_)]() mutable {
                            std::move(handler)(nullptr);
                    }));
                }
            } catch (std::exception &e) {

            }
        }, detached);
    }

    BaseActorContext::SessionNode::~SessionNode() {
        if (!completed_.test_and_set(std::memory_order_acquire)) {
            const auto work = asio::make_work_guard(handler_);
            const auto alloc = asio::get_associated_allocator(handler_, asio::recycling_allocator<void>());
            asio::dispatch(
                work.get_executor(),
                asio::bind_allocator(alloc, [handler = std::move(handler_)]() mutable {
                    std::move(handler)(nullptr);
                })
            );
        }
    }

    void BaseActorContext::SessionNode::dispatch(PackageHandle &&res) {
        if (!completed_.test_and_set(std::memory_order_acquire)) {
            const auto work = asio::make_work_guard(handler_);
            const auto alloc = asio::get_associated_allocator(handler_, asio::recycling_allocator<void>());

            asio::dispatch(
                work.get_executor(),
                asio::bind_allocator(
                    alloc,
                    [handler = std::move(handler_), response = std::move(res)]() mutable {
                        std::move(handler)(std::move(response));
                    }
                )
            );

            timer_.cancel();
        }
    }

    void BaseActorContext::SessionNode::cancel() {
        timer_.cancel();
    }

    BaseActorContext::BaseActorContext(asio::io_context &ctx, ActorHandle &&handle)
        : ctx_(ctx),
          handle_(std::move(handle)),
          mailbox_(ctx_, 1024),
          ticker_(ctx_) {
#ifndef NDEBUG
        assert(handle_ != nullptr);
#endif
        handle_->onInitial(this);
    }

    BaseActorContext::~BaseActorContext() {

    }

    AttributeMap &BaseActorContext::attr() {
        return attr_;
    }

    const AttributeMap &BaseActorContext::attr() const {
        return attr_;
    }

    void BaseActorContext::run(DataAssetHandle &&data) {
        handle_->onStart(data.get());
        co_spawn(ctx_, [self = shared_from_this()]() mutable -> awaitable<void> {
            if (self->handle_->enableTick_) {
                co_await(
                  self->process() &&
                  self->tick()
                );
            } else {
                co_await self->process();
            }
        }, detached);
    }

    void BaseActorContext::terminate() {
        if (!mailbox_.is_open())
            return;

        running_.clear();

        ticker_.cancel();
        mailbox_.cancel();
        mailbox_.close();
    }

    bool BaseActorContext::isRunning() const {
        return mailbox_.is_open() && running_.test();
    }

    BaseActor *BaseActorContext::getActor() const {
        if (handle_)
            return handle_.get();

        return nullptr;
    }

    void BaseActorContext::pushEvent(
        const int64_t evt,
        unique_ptr<DataAsset> &&data
    ) {
        Envelope evl(0, 0, evt, std::move(data));
        this->pushEnvelope(std::move(evl));
    }

    void BaseActorContext::pushEnvelope(Envelope &&envelope) {
        if (!isRunning())
            return;

        mailbox_.try_send_via_dispatch(std::error_code{}, std::move(envelope));
    }

    void BaseActorContext::createSession(
        int ty,
        const int64_t target,
        PackageHandle &&req,
        SessionHandler &&handle
    ) {
        if (!isRunning()) {
            const auto work = asio::make_work_guard(handle);
            const auto alloc = asio::get_associated_allocator(handle, asio::recycling_allocator<void>());
            asio::dispatch(
                work.get_executor(),
                asio::bind_allocator(alloc, [handler = std::move(handle)]() mutable {
                    std::move(handler)(nullptr);
                })
            );
            return;
        }

        const auto sess = sessAlloc_.allocate();

        // 判断会话id是否合法
        {
            unique_lock lock(sessMutex_);

            if (sessions_.contains(sess)) {
                lock.unlock();

                const auto work = asio::make_work_guard(handle);
                const auto alloc = asio::get_associated_allocator(handle, asio::recycling_allocator<void>());
                asio::dispatch(
                    work.get_executor(),
                    asio::bind_allocator(alloc, [handler = std::move(handle)]() mutable {
                        std::move(handler)(nullptr);
                    })
                );

                return;
            }

            // 创建新的会话回调节点
            sessions_.emplace(sess, make_shared<SessionNode>(ctx_, std::move(handle), sess));
        }

        ty |= Envelope::kRequest;
        sendRequest(ty, sess, target, std::move(req));
    }

    awaitable<void> BaseActorContext::process() {
        running_.test_and_set();
        try {
            while (isRunning()) {
                // 从邮箱中读取一条信息
                auto [ec, evl] = co_await mailbox_.async_receive();

                if (ec == asio::error::operation_aborted ||
                    ec == asio::experimental::error::channel_closed) {
                    break;
                }

                if (ec) {
                    // onErrorCode(ec);
                    break;
                }

                switch (evl.tag) {
                    case Envelope::kPackage: {
                        auto *pkg = std::get_if<PackageHandle>(&evl.variant);

                        // 异步请求处理
                        if ((evl.type & Envelope::kRequest) != 0) {
                            const auto sess = evl.session;
                            const auto from = evl.source;
                            int type = Envelope::kResponse;

                            if ((evl.type & Envelope::kFromPlayer) != 0) {
                                type |= Envelope::kToPlayer;
                            }
                            if ((evl.type & Envelope::kFromService) != 0) {
                                type |= Envelope::kToService;
                            }

                            if (pkg != nullptr) {
                                auto res = handle_->onRequest(evl.source, std::move(*pkg));
                                sendResponse(type, sess, from, std::move(res));
                            }
                        }
                        // 异步请求返回
                        else if ((evl.type & Envelope::kResponse) != 0) {
                            shared_ptr<SessionNode> node = nullptr;

                            // 查找会话节点
                            {
                                unique_lock lock(sessMutex_);
                                if (const auto it = sessions_.find(evl.session); it != sessions_.end()) {
                                    node = it->second;
                                    sessions_.erase(it);
                                }
                            }

                            if (node != nullptr) {
                                if (pkg != nullptr) {
                                    node->dispatch(std::move(*pkg));
                                } else {
                                    node->cancel();
                                }
                                sessAlloc_.recycle(node->sess_);
                            }
                        }
                        // 普通信息
                        else {
                            if (pkg != nullptr) {
                                handle_->onRequest(evl.source, std::move(*pkg));
                            }
                        }
                    }
                    break;
                    case Envelope::kDataAsset: {
                        if (const auto *da = std::get_if<DataAssetHandle>(&evl.variant)) {
                            handle_->onEvent(evl.source, evl.event, da->get());
                        }
                    }
                    break;
                    case Envelope::kTickInfo: {
                        if (const auto *info = std::get_if<ActorTickInfo>(&evl.variant)) {
                            handle_->onTick(info->now, info->delta);
                        }
                    }
                    break;
                    case Envelope::kTask: {
                        if (auto *task = std::get_if<ActorTask>(&evl.variant)) {
                            std::invoke(*task, handle_.get());
                        }
                    }
                    break;
                    default: break;
                }
            }

            // 释放所有会话
            for (const auto &node: sessions_ | std::views::values) {
                node->cancel();
            }

            running_.clear();
            sessions_.clear();

            // Call actor terminate
            handle_->onTerminate();
        } catch (std::exception &e) {
            // onException(e);
        }
    }

    awaitable<void> BaseActorContext::tick() {
        try {
            auto point = std::chrono::steady_clock::now();
            constexpr SteadyDuration kTickDelta = std::chrono::microseconds(500);
            while (isRunning()) {
                point += kTickDelta;
                ticker_.expires_at(point);

                const auto [ec] = co_await ticker_.async_wait();
                if (ec) {
                    //
                    break;
                }

                Envelope evl(point, kTickDelta);
                this->pushEnvelope(std::move(evl));
            }
        } catch (std::exception &e) {

        }
    }
}
