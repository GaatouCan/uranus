#include "GamePlayer.h"
#include "actor/ActorContext.h"
#include "actor/BaseActorContext.h"
#include "actor/Envelope.h"

#include <login/DA_PlayerResult.h>
#include <logger/LoggerModule.h>
#include <database/DatabaseModule.h>

namespace gameplay {

    using uranus::actor::Package;
    using uranus::actor::Envelope;
    using uranus::logger::LoggerModule;
    using uranus::database::DatabaseModule;
    using uranus::actor::BaseActorContext;

    GamePlayer::GamePlayer()
        : component_(*this) {
    }

    GamePlayer::~GamePlayer() {
    }

    void GamePlayer::onInitial(ActorContext *ctx) {
        super::onInitial(ctx);

        if (auto *module = ACTOR_GET_MODULE(LoggerModule); module != nullptr) {
            module->createLogger("game_player", "player");
        }
    }

    void GamePlayer::onStart(DataAsset *data) {
        if (data != nullptr) {
            if (const auto *temp = dynamic_cast<uranus::login::DA_PlayerResult *>(data)) {
                component_.deserialize(temp->data);
            }
        }

        this->onLogin();
    }

    void GamePlayer::onTerminate() {
    }

    void GamePlayer::save() {

        nlohmann::json data;
        data["_id"] = getPlayerId();

        component_.serialize(data);

        // TODO: Send to database
    }

    void GamePlayer::onLogin() {
        component_.onLogin();
    }

    void GamePlayer::onLogout() {
        component_.onLogout();
    }

    void GamePlayer::sendToClient(PackageHandle &&pkg) const {
        getContext()->send(Envelope::kToClient, 0, std::move(pkg));
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

    ComponentModule &GamePlayer::getComponentModule() {
        return component_;
    }
}

using uranus::actor::BasePlayer;
using gameplay::GamePlayer;

ACTOR_EXPORT BasePlayer *CreatePlayer() {
    return new GamePlayer();
}

ACTOR_EXPORT void DeletePlayer(BasePlayer *plr) {
    delete plr;
}
