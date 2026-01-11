#include "GamePlayer.h"

namespace gameplay {
    GamePlayer::GamePlayer() {
    }

    GamePlayer::~GamePlayer() {
    }

    void GamePlayer::onInitial(ActorContext *ctx) {
        super::onInitial(ctx);
    }

    void GamePlayer::onTerminate() {

    }

    void GamePlayer::onPackage(PackageHandle &&pkg) {
    }

    PackageHandle GamePlayer::onRequest(PackageHandle &&pkg) {
        // TODO
        return nullptr;
    }
}

using uranus::actor::BasePlayer;

ACTOR_EXPORT BasePlayer *CreatePlayer() {
    return new gameplay::GamePlayer();
}

ACTOR_EXPORT void DeletePlayer(BasePlayer *plr) {
    delete plr;
}