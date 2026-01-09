#include "BaseActorContext.h"
#include "BaseActor.h"

#include <asio/detached.hpp>
#include <ranges>

namespace uranus::actor {
    BaseActorContext::SessionNode::SessionNode(SessionHandle &&h, uint32_t s)
        : handle(std::move(h)),
          work(asio::make_work_guard(handle)),
          sess(s) {
    }

    BaseActorContext::BaseActorContext(asio::io_context &ctx)
        : ctx_(ctx),
          mailbox_(ctx_, 1024),
          id_(0) {
    }

    BaseActorContext::~BaseActorContext() {

    }

    void BaseActorContext::setId(const uint32_t id) {
        id_ = id;
    }

    uint32_t BaseActorContext::getId() const {
        return id_;
    }

    void BaseActorContext::setUpActor(ActorHandle &&handle) {
        if (!handle)
            return;

        if (handle_)
            return;

        handle_ = std::move(handle);
        handle_->setContext(this);
    }

    BaseActor *BaseActorContext::getActor() const {
        return handle_.get();
    }

    void BaseActorContext::run() {
        if (!handle_)
            throw std::runtime_error("ActorContext::run - Actor is null");

        // Call actor initial
        handle_->onInitial();

        co_spawn(ctx_, [self = shared_from_this()]() -> awaitable<void> {
            co_await self->process();
        }, asio::detached);
    }

    void BaseActorContext::terminate() {
        if (!mailbox_.is_open())
            return;

        mailbox_.cancel();
        mailbox_.close();
    }

    AttributeMap &BaseActorContext::attr() {
        return attr_;
    }

    bool BaseActorContext::isRunning() const {
        return mailbox_.is_open();
    }

    void BaseActorContext::pushEnvelope(Envelope &&envelope) {
        if (!isRunning())
            return;

        mailbox_.try_send_via_dispatch(std::error_code{}, std::move(envelope));
    }

    void BaseActorContext::onErrorCode(std::error_code ec) {
    }

    void BaseActorContext::onException(std::exception &e) {
    }

    awaitable<void> BaseActorContext::process() {
        try {
            while (isRunning()) {
                // 从邮箱中读取一条信息
                auto [ec, envelope] = co_await mailbox_.async_receive();

                if (ec == asio::error::operation_aborted ||
                    ec == asio::experimental::error::channel_closed) {
                    break;
                }

                if (ec) {
                    onErrorCode(ec);
                    break;
                }

                // 异步请求处理
                if ((envelope.type & Package::kRequest) != 0) {
                    const auto sess = envelope.session;
                    const auto from = envelope.source;
                    int type = Package::kResponse;

                    if ((envelope.type & Package::kFromPlayer) != 0) {
                        type |= Package::kToPlayer;
                    }
                    if ((envelope.type & Package::kFromService) != 0) {
                        type |= Package::kToService;
                    }

                    auto res = handle_->onRequest(std::move(envelope));
                    sendResponse(type, sess, from, std::move(res));
                }
                // 异步请求返回
                else if ((envelope.type & Package::kResponse) != 0) {
                    unique_ptr<SessionNode> node = nullptr;

                    // 查找会话节点
                    {
                        unique_lock lock(sessMutex_);
                        if (const auto it = sessions_.find(envelope.session); it != sessions_.end()) {
                            node = std::move(it->second);
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

                        sessAlloc_.recycle(node->sess);
                    }
                }
                // 普通信息
                else {
                    handle_->onPackage(std::move(envelope));
                }
            }

            // 释放所有会话
            for (auto it = sessions_.begin(); it != sessions_.end();) {
                if (it->second == nullptr) {
                    ++it;
                    continue;
                }

                const auto node = std::move(it->second);

                auto alloc = asio::get_associated_allocator(node->handle, asio::recycling_allocator<void>());
                asio::dispatch(
                    node->work.get_executor(),
                    asio::bind_allocator(alloc, [handler = std::move(node->handle)]() mutable {
                        std::move(handler)(nullptr);
                    })
                );

                it = sessions_.erase(it);
            }

            sessions_.clear();

            // Call actor terminate
            handle_->onTerminate();
        } catch (std::exception &e) {
            onException(e);
        }
    }
}
