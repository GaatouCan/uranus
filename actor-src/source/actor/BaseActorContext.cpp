#include "BaseActorContext.h"
#include "BaseActor.h"

#include <asio/detached.hpp>
#include <asio/bind_allocator.hpp>
#include <ranges>

namespace uranus::actor {
    using std::make_unique;

    // BaseActorContext::SessionNode::SessionNode(SessionHandle &&h, uint32_t s)
    //     : handle(std::move(h)),
    //       work(asio::make_work_guard(handle)),
    //       sess(s) {
    // }
    //

    // void BaseActorContext::onErrorCode(std::error_code ec) {
    // }
    //
    // void BaseActorContext::onException(std::exception &e) {
    // }


    BaseActorContext::BaseActorContext(asio::io_context &ctx, ActorHandle &&handle)
        : ctx_(ctx),
          strand_(asio::make_strand(ctx_)),
          handle_(std::move(handle)),
          mailbox_(ctx_, 1024) {
        if (handle_ == nullptr)
            std::abort();

        handle_->onInitial(this);
    }

    BaseActorContext::~BaseActorContext() {
    }

    AttributeMap &BaseActorContext::attr() {
        return attr_;
    }

    void BaseActorContext::run() {
        co_spawn(ctx_, [self = shared_from_this()]() mutable -> awaitable<void> {
            co_await self->process();
        }, asio::detached);
    }

    void BaseActorContext::terminate() {
        if (!mailbox_.is_open())
            return;

        mailbox_.cancel();
        mailbox_.close();
    }

    bool BaseActorContext::isRunning() const {
        return mailbox_.is_open();
    }

    BaseActor *BaseActorContext::getActor() const {
        if (handle_)
            return handle_.get();

        return nullptr;
    }

    void BaseActorContext::pushEnvelope(Envelope &&envelope) {
        if (!isRunning())
            return;

        mailbox_.try_send_via_dispatch(std::error_code{}, std::move(envelope));
    }

    void BaseActorContext::createSession(int ty, int64_t target, PackageHandle &&req, SessionHandle &&handle) {
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

               //  co_spawn(work.get_executor())
            }

            // 创建新的会话回调节点
            // sessions_.emplace(sess, make_unique<SessionNode>(std::move(handle), sess));
        }
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
                    // onErrorCode(ec);
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

                    auto res = handle_->onRequest(std::move(envelope.package));
                    sendResponse(type, sess, from, std::move(res));
                }
                // 异步请求返回
                else if ((envelope.type & Package::kResponse) != 0) {
                    // unique_ptr<SessionNode> node = nullptr;
                    //
                    // // 查找会话节点
                    // {
                    //     unique_lock lock(sessMutex_);
                    //     if (const auto it = sessions_.find(envelope.session); it != sessions_.end()) {
                    //         node = std::move(it->second);
                    //         sessions_.erase(it);
                    //     }
                    // }
                    //
                    // if (node != nullptr) {
                    //     const auto work = asio::make_work_guard(node->handle);
                    //     const auto alloc = asio::get_associated_allocator(node->handle, asio::recycling_allocator<void>());
                    //     asio::dispatch(
                    //         work.get_executor(),
                    //         asio::bind_allocator(
                    //             alloc,
                    //             [handler = std::move(node->handle), res = std::move(envelope.package)]() mutable {
                    //                 std::move(handler)(std::move(res));
                    //             }
                    //         )
                    //     );
                    //
                    //     sessAlloc_.recycle(node->sess);
                    // }
                }
                // 普通信息
                else {
                    handle_->onPackage(std::move(envelope.package));
                }
            }

            // 释放所有会话
            // for (auto it = sessions_.begin(); it != sessions_.end();) {
            //     if (it->second == nullptr) {
            //         ++it;
            //         continue;
            //     }
            //
            //     const auto node = std::move(it->second);
            //
            //     auto alloc = asio::get_associated_allocator(node->handle, asio::recycling_allocator<void>());
            //     asio::dispatch(
            //         node->work.get_executor(),
            //         asio::bind_allocator(alloc, [handler = std::move(node->handle)]() mutable {
            //             std::move(handler)(nullptr);
            //         })
            //     );
            //
            //     it = sessions_.erase(it);
            // }
            //
            // sessions_.clear();

            // Call actor terminate
            handle_->onTerminate();
        } catch (std::exception &e) {
            // onException(e);
        }
    }
}
