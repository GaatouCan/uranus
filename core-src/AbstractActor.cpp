#include "AbstractActor.h"
#include "ActorContext.h"

#include <format>
#include <spdlog/spdlog.h>


namespace uranus {
    AbstractActor::AbstractActor()
        : ctx_(nullptr) {
    }

    AbstractActor::~AbstractActor() {
    }

    int AbstractActor::Initial(DataAsset *data) {
        return 1;
    }

    int AbstractActor::Start() {
        return 1;
    }

    void AbstractActor::Stop() {
    }

    void AbstractActor::OnReceive(const Message &msg) {
    }

    void AbstractActor::OnRequest(const Message &msg, Message &res) {
    }

    ActorContext *AbstractActor::GetActorContext() const {
        if (ctx_ == nullptr) {
            throw std::logic_error(
                std::format("Call ::GetActorContext() before ::SetUpContext(ActorContext *), ActorContext[{:p}]",
                            static_cast<const void *>(this)));
        }

        return ctx_;
    }

    GameServer *AbstractActor::GetGameServer() const {
        return this->GetActorContext()->GetGameServer();
    }

    void AbstractActor::SetUpContext(ActorContext *ctx) {
        if (ctx_ != nullptr) {
            SPDLOG_WARN("Actor[{:p}] - Try to set up context again",
                static_cast<void *>(this));
        }
        ctx_ = ctx;
    }
}
