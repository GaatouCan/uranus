#include "GamePlayer.h"
#include "actor/ActorContext.h"

#include <logger/LoggerModule.h>

namespace gameplay {

    using uranus::actor::Package;
    using uranus::logger::LoggerModule;

    GamePlayer::GamePlayer() {
    }

    GamePlayer::~GamePlayer() {
    }

    void GamePlayer::onInitial(ActorContext *ctx) {
        super::onInitial(ctx);

        if (auto *module = ACTOR_GET_MODULE(LoggerModule); module != nullptr) {
            module->createLogger("game_player", "player");
        }
    }

    void GamePlayer::onStart() {
        const auto logger = spdlog::get("game_player");
        logger->info("Player[{}] login", getPlayerId());
    }

    void GamePlayer::onTerminate() {

    }

    void GamePlayer::sendToClient(PackageHandle &&pkg) const {
        getContext()->send(Package::kToClient, 0, std::move(pkg));
    }

    int64_t GamePlayer::getPlayerId() const {
        auto *ctx = getContext();
        if (ctx == nullptr)
            return -1;

        if (const auto op = ctx->attr().get<int64_t>("PLAYER_ID"); op.has_value()) {
            return op.value();
        }

        return -1;
    }
}

using uranus::actor::BasePlayer;

ACTOR_EXPORT BasePlayer *CreatePlayer() {
    return new gameplay::GamePlayer();
}

ACTOR_EXPORT void DeletePlayer(BasePlayer *plr) {
    delete plr;
}