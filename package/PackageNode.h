#pragma once

#include "MessageNode.h"


namespace uranus {

    class Message;

    namespace network {

        class PackageNode final : public MessageNode {

        public:
            PackageNode();
            ~PackageNode() override;

            // void Execute(ActorContext *ctx) override;
        };
    }
}
