#include "GamePlayer.h"
#include "actor/ActorContext.h"
#include "actor/BaseActorContext.h"
#include "actor/Envelope.h"

#include "../common/ProtocolID.h"

#include <database/DA_PlayerResult.h>
#include <logger/LoggerModule.h>
#include <database/DatabaseModule.h>

#include <greeting.pb.h>


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
            if (const auto *temp = dynamic_cast<uranus::database::DA_PlayerResult *>(data)) {
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

        greeting::SyncPlayerInfo info;

        {
            const auto &comp = component_.getAppearance();
            info.set_current_avatar(comp.getCurrentAvatar());
            info.set_current_frame(comp.getCurrentFrame());
            info.set_current_background(comp.getCurrentBackground());
        }

        // TODO: Other Info

        sendToService("FriendService", protocol::kSyncPlayerInfo, info);
    }

    void GamePlayer::onLogout() {
        component_.onLogout();
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

EXPORT_ACTOR_VERSION

ACTOR_EXPORT BasePlayer *CreatePlayer() {
    return new GamePlayer();
}

ACTOR_EXPORT void DeletePlayer(BasePlayer *plr) {
    delete plr;
}
