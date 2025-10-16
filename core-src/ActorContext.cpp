#include "ActorContext.h"
#include "AbstractActor.h"
#include "GameServer.h"


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
            node->CleanUp(this);
        }

        while (true) {
            const auto [ec, node] = co_await channel_.async_receive();
            if (ec == asio::error::operation_aborted) {
                break;
            }

            if (node != nullptr)
                node->CleanUp(this);
        }
    } catch (const std::exception &e) {
        // TODO:
    }
}

void ActorContext::CleanUp() {
}
