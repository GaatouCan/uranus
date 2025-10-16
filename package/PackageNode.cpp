#include "PackageNode.h"
#include "Message.h"
#include "Package.h"

namespace uranus::network {
    PackageNode::PackageNode() {
    }

    PackageNode::~PackageNode() {
        if (msg_ && msg_->data != nullptr) {
            auto *pkg = static_cast<Package *>(msg_->data);
            pkg->Recycle();
        }
    }

    // void PackageNode::Execute(ActorContext *ctx) {
    //     if (!msg_) {
    //         ctx->GetActor()->OnReceive(msg_);
    //     }
    // }
}
