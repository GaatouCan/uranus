#include "MessageNode.h"
#include "AbstractActor.h"
#include "ActorContext.h"
#include "Message.h"

namespace uranus {
    MessageNode::MessageNode()
        : msg_(nullptr) {
    }

    MessageNode::~MessageNode() {
        delete msg_;
    }

    void MessageNode::SetMessage(Message *msg) {
        msg_ = msg;
    }

    void MessageNode::Execute(ActorContext *ctx) {
        if (msg_ == nullptr)
            return;

        if (msg_->type & Message::kRequest) {
            ctx->OnRequest(msg_);
        } else if (msg_->type & Message::kResponse) {
            ctx->OnResponse(msg_);
            msg_ = nullptr;
        } else {
            ctx->GetActor()->OnReceive(msg_);
        }
    }
}
