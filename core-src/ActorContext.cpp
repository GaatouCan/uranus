#include "ActorContext.h"
#include "AbstractActor.h"
#include "GameServer.h"

#include <ranges>
#include <spdlog/spdlog.h>


namespace uranus {
    ActorContext::ActorContext(GameServer *ser)
        : server_(ser),
          ctx_(server_->GetWorkerIOContext()),
          sess_id_alloc_() {
    }

    ActorContext::~ActorContext() {
    }

    GameServer *ActorContext::GetGameServer() const {
        return server_;
    }

    asio::io_context &ActorContext::GetIOContext() {
        return ctx_;
    }

    int ActorContext::Initial(DataAsset *data) {
        return 1;
    }

    int ActorContext::Start() {
        return 1;
    }

    void ActorContext::Stop() {
        if (channel_ != nullptr && channel_->is_open()) {
            channel_->cancel();
            channel_->close();
        }
    }

    void ActorContext::PushMessage(const Message &msg) {
        if (!channel_->is_open()) {
            this->DisposeMessage(msg);
            return;
        }

        if (!channel_->try_send_via_dispatch(error_code{}, msg)) {
            // If the channel is full
            // Resend the message in coroutine
            co_spawn(ctx_, [self = shared_from_this(), this, msg]() mutable -> awaitable<void> {
                const auto [ec] = co_await channel_->async_send(error_code{}, msg);
                if (ec == asio::experimental::error::channel_closed ||
                    ec == asio::error::operation_aborted) {
                    this->DisposeMessage(msg);
                }
            }, detached);
        }
    }

    void ActorContext::SetUpActor() {
        if (auto *actor = this->GetActor(); actor != nullptr) {
            actor->SetUpContext(this);
        }
    }

    bool ActorContext::IsChannelClosed() const {
        return channel_ == nullptr || !channel_->is_open();
    }

    int32_t ActorContext::AllocateSessionID() {
        int32_t id = sess_id_alloc_.Allocate();

        while (sessions_.contains(id)) {
            id = sess_id_alloc_.Allocate();
        }

        return id;
    }

    void ActorContext::RecycleSessionID(const int32_t id) {
        sess_id_alloc_.Recycle(id);
    }

    void ActorContext::PushSession(const int32_t id, SessionNode &&node) {
        sessions_.emplace(id, std::move(node));
    }

    std::optional<ActorContext::SessionNode> ActorContext::TakeSession(const int32_t id) {
        const auto it = sessions_.find(id);

        if (it == sessions_.end()) {
            return std::nullopt;
        }

        auto node = std::move(it->second);
        sessions_.erase(it);

        return node;
    }

    awaitable<void> ActorContext::Process() {
        try {
            while (channel_->is_open()) {
                const auto [ec, msg] = co_await channel_->async_receive();
                if (ec == asio::experimental::error::channel_closed ||
                    ec == asio::error::operation_aborted) {
                    SPDLOG_DEBUG("Actor[{:p}] close channel", static_cast<void *>(this));
                    this->DisposeMessage(msg);
                    break;
                }

                if (ec) {
                    SPDLOG_ERROR("Actor[{:p}] error in processing: {}", static_cast<void *>(this), ec.message());
                    this->DisposeMessage(msg);
                    continue;
                }

                // ::HandleMessage must manage the resource in msg
                this->HandleMessage(msg);
            }
        } catch (const std::exception &e) {
            SPDLOG_ERROR("Actor[{:p}] - Exception: {}", static_cast<void *>(this), e.what());
        }

        // Clean the channel
        while (channel_->try_receive([this](auto, const auto &msg) {
            this->DisposeMessage(msg);
        })) {}
    }

    void ActorContext::CleanUp() {
        // Clean the session still not replied
        for (const auto &sess : sessions_ | std::views::values) {
            auto alloc = asio::get_associated_allocator(
                sess.handler,
                asio::recycling_allocator<void>()
            );

            asio::dispatch(
                sess.work.get_executor(),
                asio::bind_allocator(
                    alloc,
                    [handler = sess.handler]() mutable {
                        std::move(handler)(std::nullopt);
                    }
                )
            );
        }
        sessions_.clear();
    }
}
