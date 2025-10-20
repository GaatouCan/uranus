#include "AbstractActor.h"
#include "ActorContext.h"

#include <format>

namespace uranus {
    AbstractActor::AbstractActor()
        : ctx_(nullptr) {
    }

    AbstractActor::~AbstractActor() {
    }

    int AbstractActor::Initial(DataAsset *data) {
        return 0;
    }

    int AbstractActor::Start() {
        return 0;
    }

    void AbstractActor::Stop() {
    }

    void AbstractActor::OnReceive(Message *msg) {
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
        ctx_ = ctx;
    }
}