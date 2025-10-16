#include "PackageNode.h"
#include "Message.h"
#include "Package.h"

namespace uranus::network {
    PackageNode::~PackageNode() {
        if (msg_ && msg_->data != nullptr) {
            auto *pkg = static_cast<Package *>(msg_->data);
            pkg->Recycle();
        }
    }
}
