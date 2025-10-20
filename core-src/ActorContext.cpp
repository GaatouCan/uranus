#include "ActorContext.h"
#include "AbstractActor.h"
#include "GameServer.h"
#include "ChannelNode.h"

#include <spdlog/spdlog.h>


namespace uranus {
    ActorContext::ActorContext(GameServer *ser)
        : server_(ser),
          ctx_(server_->GetWorkerIOContext()),
          channel_(ctx_, 1024) {
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
        if (channel_.is_open())
            channel_.close();
    }

    void ActorContext::SetUpActor() {
        if (auto *actor = this->GetActor(); actor != nullptr) {
            actor->SetUpContext(this);
        }
    }

    bool ActorContext::IsChannelClosed() const {
        return !channel_.is_open();
    }

    bool ActorContext::PushNode(unique_ptr<ChannelNode> &&node) {
        if (!channel_.is_open())
            return true;

        return channel_.try_send_via_dispatch(error_code{}, std::move(node));
    }

    void ActorContext::AsyncPushNode(unique_ptr<ChannelNode> &&node) {
        if (!channel_.is_open())
            return;

        co_spawn(ctx_, [node = std::move(node), self = shared_from_this()]() mutable -> awaitable<void> {
            co_await self->channel_.async_send(error_code{}, std::move(node));
        }, detached);
    }

    awaitable<void> ActorContext::Process() {
        try {
            while (channel_.is_open()) {
                const auto [ec, node] = co_await channel_.async_receive();
                if (ec == asio::experimental::error::channel_closed) {
                    SPDLOG_DEBUG("{} - Actor[{:p}] close channel",
                        __FUNCTION__, static_cast<void *>(this));
                    break;
                }

                if (node == nullptr)
                    continue;

                node->Execute(this);
            }
        } catch (const std::exception &e) {
            SPDLOG_ERROR("{} - Actor[{:p}] - Exception: {}",
                __FUNCTION__, static_cast<void *>(this), e.what());
        }
    }

    void ActorContext::CleanUp() {
    }
}