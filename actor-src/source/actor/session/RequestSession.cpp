#include "session/RequestSession.h"
#include "session/SessionManager.h"
#include "BaseActorContext.h"

#include <asio/bind_allocator.hpp>
#include <asio/detached.hpp>
#include <asio/co_spawn.hpp>

namespace uranus::actor {

    using asio::detached;
    using asio::co_spawn;
    using asio::awaitable;

    static void DispatchResponse(SessionHandler &&handler, PackageHandle &&res) {
        const auto work = asio::make_work_guard(handler);
        const auto alloc = asio::get_associated_allocator(handler, asio::recycling_allocator<void>());

        asio::dispatch(
            work.get_executor(),
            asio::bind_allocator(
                alloc,
                [handler = std::move(handler), response = std::move(res)]() mutable {
                    std::move(handler)(std::move(response));
                }
            )
        );
    }

    RequestSession::RequestSession(const shared_ptr<BaseActorContext> &owner, SessionHandler &&handler, const int64_t id)
        : exec_(owner->executor()),
          id_(id),
          owner_(owner),
          handler_(std::move(handler)),
          guard_(asio::make_work_guard(handler_)),
          timer_(exec_) {
        // 发起超时等待
        co_spawn(exec_, [self = shared_from_this()]() mutable -> awaitable<void> {
            try {
                self->timer_.expires_after(std::chrono::seconds(5));

                // 即使返回error_code也需要唤醒协程 所以直接无论如何都唤醒
                co_await self->timer_.async_wait();

                // 超时返回空指针
                if (!self->completed_.test_and_set(std::memory_order_acquire)) {
                    DispatchResponse(std::move(self->handler_), nullptr);
                    if (const auto temp = self->owner_.lock()) {
                        temp->sessionManager_.removeOnComplete(self->id_);
                    }
                }
            } catch (std::exception &e) {
                if (const auto temp = self->owner_.lock()) {
                    temp->onException(e);
                }
            }
        }, detached);

    }

    RequestSession::~RequestSession() {
        this->cancel();
    }

    void RequestSession::dispatch(PackageHandle &&res) {
        if (!completed_.test_and_set(std::memory_order_acquire)) {
            // 因为completed_已经被设置 所以timer.async_wait下面的分支不会执行
            timer_.cancel();

            DispatchResponse(std::move(handler_), std::move(res));
            if (const auto owner = owner_.lock()) {
                owner->sessionManager_.removeOnComplete(id_);
            }
        }
    }

    void RequestSession::cancel() {
        this->dispatch(nullptr);
    }
}
