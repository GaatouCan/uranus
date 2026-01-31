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

#define EXPORT_PLAYER(plr)                                          \
EXPORT_ACTOR_VERSION                                                \
ACTOR_EXPORT uranus::actor::BasePlayer *CreatePlayer() {            \
    return new plr();                                               \
}                                                                   \
ACTOR_EXPORT void DeletePlayer(uranus::actor::BasePlayer *ptr) {    \
    delete ptr;                                                     \
}