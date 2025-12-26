#include "GamePlayer.h"

namespace uranus {
    void GamePlayer::onPackage(Envelope &&envelope) {
    }

    PackageHandle GamePlayer::onRequest(Envelope &&envelope) {
        // TODO
        return nullptr;
    }
}

using uranus::actor::BasePlayer;

ACTOR_EXPORT BasePlayer *CreatePlayer() {
    return new uranus::GamePlayer();
}

ACTOR_EXPORT void DeletePlayer(BasePlayer *plr) {
    delete plr;
}
