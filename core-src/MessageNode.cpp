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
        if (msg_) {
            ctx->GetActor()->OnReceive(msg_);
        }
    }
}
