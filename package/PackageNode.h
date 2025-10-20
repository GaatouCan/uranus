#pragma once

#include "MessageNode.h"

namespace uranus::network {
    class NETWORK_API PackageNode final : public MessageNode {
    public:
        ~PackageNode() override;
    };
}
