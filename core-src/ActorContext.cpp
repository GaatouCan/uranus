#include "ActorContext.h"
#include "AbstractActor.h"
#include "GameServer.h"
#include "ChannelNode.h"


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

    void ActorContext::SetUpActor() {
        if (auto *actor = this->GetActor(); actor != nullptr) {
            actor->SetUpContext(this);
        }
    }

    void ActorContext::PushNode(unique_ptr<ChannelNode> &&node) {
        if (!channel_.is_open())
            return;

        if (!channel_.try_send_via_dispatch(error_code{}, std::move(node))) {
            // TODO
        }
    }

    awaitable<void> ActorContext::Process() {
        try {
            while (channel_.is_open()) {
                const auto [ec, node] = co_await channel_.async_receive();
                if (ec) {
                    if (ec == asio::error::operation_aborted) {
                        // TODO
                        break;
                    }
                }

                if (node == nullptr) continue;

                node->Execute(this);
            }

            while (true) {
                const auto [ec, node] = co_await channel_.async_receive();
                if (ec == asio::error::operation_aborted)
                    break;
            }
        } catch (const std::exception &e) {
            // TODO:
        }
    }

    void ActorContext::CleanUp() {
    }
}