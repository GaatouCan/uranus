#pragma once

#include "BaseActor.h"

namespace uranus::actor {
    class ACTOR_API BasePlayer : public BaseActor {

    public:
        BasePlayer();
        ~BasePlayer() override;

        void sendToClient(PackageHandle &&pkg) const;
        void sendToService(const std::string &name, PackageHandle &&pkg) const;
    };
}