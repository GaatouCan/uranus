#include "ActorContext.h"
#include "AbstractActor.h"
#include "GameServer.h"
#include "ChannelNode.h"

#include <spdlog/spdlog.h>

#include "Message.h"


namespace uranus {
    ActorContext::ActorContext(GameServer *ser)
        : server_(ser),
          ctx_(server_->GetWorkerIOContext()) {
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

    void ActorContext::SetUpActor() {
        if (auto *actor = this->GetActor(); actor != nullptr) {
            actor->SetUpContext(this);
        }
    }

    bool ActorContext::IsChannelClosed() const {
        return channel_ == nullptr || !channel_->is_open();
    }

    void ActorContext::PushNode(unique_ptr<ChannelNode> &&node) const {
        if (!channel_->is_open()) {
            // delete node;
            return;
        }

        // if (!channel_->try_send_via_dispatch(error_code{}, node)) {
        //     co_spawn(ctx_, [self = shared_from_this(), node]() mutable -> awaitable<void> {
        //         const auto [ec] = co_await self->channel_->async_send(error_code{}, node);
        //         if (ec == asio::experimental::error::channel_closed) {
        //             delete node;
        //         }
        //     }, detached);
        // }
        channel_->try_send_via_dispatch(error_code{}, std::move(node));
    }

    int32_t ActorContext::AllocateSessionID() {
        int32_t id = sess_id_alloc_.Allocate();

        {
            shared_lock lock(sess_mutex_);
            while (sessions_.contains(id)) {
                id = sess_id_alloc_.Allocate();
            }
        }

        return id;
    }

    void ActorContext::RecycleSessionID(const int32_t id) {
        sess_id_alloc_.Recycle(id);
    }

    void ActorContext::PushSession(const int32_t id, unique_ptr<SessionNode> &&node) {
        unique_lock lock(sess_mutex_);
        sessions_.insert_or_assign(id, std::move(node));
    }

    unique_ptr<ActorContext::SessionNode> ActorContext::TakeSession(const int32_t id) {
        unique_lock lock(sess_mutex_);

        const auto it = sessions_.find(id);

        if (it == sessions_.end()) {
            return nullptr;
        }

        auto node = std::move(it->second);
        sessions_.erase(it);

        return std::move(node);
    }

    awaitable<void> ActorContext::Process() {
        try {
            while (channel_->is_open()) {
                const auto [ec, node] = co_await channel_->async_receive();
                if (ec == asio::experimental::error::channel_closed ||
                    ec == asio::error::operation_aborted) {
                    SPDLOG_DEBUG("Actor[{:p}] close channel", static_cast<void *>(this));
                    break;
                }

                if (ec) {
                    SPDLOG_ERROR("Actor[{:p}] error in processing: {}", static_cast<void *>(this), ec.message());
                    continue;
                }

                if (node == nullptr)
                    continue;

                node->Execute(this);
            }
        } catch (const std::exception &e) {
            SPDLOG_ERROR("Actor[{:p}] - Exception: {}", static_cast<void *>(this), e.what());
        }

        // Clean the channel
        while (channel_->try_receive([](auto, auto) {})) {}
    }

    void ActorContext::CleanUp() {
    }

    // void ActorContext::PushSession(int64_t target, Message *req, unique_ptr<SessionNode> &&node) {
    //     if (req == nullptr || req->data == nullptr) {
    //         delete req;
    //         auto alloc = asio::get_associated_allocator(node->handler, asio::recycling_allocator<void>());
    //         asio::dispatch(node->work.get_executor(),asio::bind_allocator(alloc, [handler = std::move(node->handler)]() mutable{
    //             std::move(handler)(nullptr);
    //         }));
    //     }
    // }
}
