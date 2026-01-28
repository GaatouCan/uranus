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

#define EXPORT_CREATE_PLAYER(plr)                           \
ACTOR_EXPORT uranus::actor::BasePlayer *CreatePlayer() {    \
    return new plr();                                       \
}

#define EXPORT_DELETE_PLAYER                                        \
ACTOR_EXPORT void DeletePlayer(uranus::actor::BasePlayer *plr) {    \
    delete plr;                                                     \
}