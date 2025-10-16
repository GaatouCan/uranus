#pragma once

#include "MessageNode.h"

namespace uranus::network {
    class PackageNode final : public MessageNode {
    public:
        ~PackageNode() override;
    };
}
