#include "GamePlayer.h"
#include "actor/ActorContext.h"

namespace gameplay {

    using uranus::actor::Package;

    GamePlayer::GamePlayer() {
    }

    GamePlayer::~GamePlayer() {
    }

    void GamePlayer::onInitial(ActorContext *ctx) {
        super::onInitial(ctx);
    }

    void GamePlayer::onTerminate() {

    }

    void GamePlayer::sendToClient(PackageHandle &&pkg) const {
        getContext()->send(Package::kToClient, 0, std::move(pkg));
    }
}

using uranus::actor::BasePlayer;

ACTOR_EXPORT BasePlayer *CreatePlayer() {
    return new gameplay::GamePlayer();
}

ACTOR_EXPORT void DeletePlayer(BasePlayer *plr) {
    delete plr;
}