#include "ActorContext.h"
#include "AbstractActor.h"
#include "GameServer.h"
#include "ChannelNode.h"

#include <spdlog/spdlog.h>


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
        if (channel_ != nullptr && channel_->is_open())
            channel_->close();
    }

    void ActorContext::SetUpActor() {
        if (auto *actor = this->GetActor(); actor != nullptr) {
            actor->SetUpContext(this);
        }
    }

    bool ActorContext::IsChannelClosed() const {
        return channel_ == nullptr || !channel_->is_open();
    }

    void ActorContext::PushNode(ChannelNode *node) {
        if (!channel_->is_open())
            return;

        if (!channel_->try_send_via_dispatch(error_code{}, node)) {
            co_spawn(ctx_, [self = shared_from_this(), node]() mutable -> awaitable<void> {
                const auto [ec] = co_await self->channel_->async_send(error_code{}, node);
                if (ec == asio::experimental::error::channel_closed) {
                    delete node;
                }
            }, detached);
        }
    }

    awaitable<void> ActorContext::Process() {
        try {
            while (channel_->is_open()) {
                const auto [ec, node] = co_await channel_->async_receive();
                if (ec == asio::experimental::error::channel_closed) {
                    SPDLOG_DEBUG("Actor[{:p}] close channel", static_cast<void *>(this));

                    delete node;
                    break;
                }

                if (node == nullptr)
                    continue;

                node->Execute(this);

                delete node;
            }

            for (;;) {
                const bool ok = channel_->try_receive([](std::error_code ec, const ChannelNode *node) {
                    delete node;
                });
                if (!ok)
                    break;
            }
        } catch (const std::exception &e) {
            SPDLOG_ERROR("Actor[{:p}] - Exception: {}", static_cast<void *>(this), e.what());
        }
    }

    void ActorContext::CleanUp() {
    }
}