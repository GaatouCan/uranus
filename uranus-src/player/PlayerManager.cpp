#include "PlayerManager.h"
#include "../GameWorld.h"

#include <actor/BasePlayer.h>


PlayerManager::PlayerManager(GameWorld &world)
    : ServerModule(world) {
}

PlayerManager::~PlayerManager() {
}

GameWorld &PlayerManager::getWorld() const {
    return dynamic_cast<GameWorld &>(getServer());
}

void PlayerManager::createPlayer(uint32_t pid, const std::string &key) {
    bool repeated = false;

    {
        std::shared_lock lock(mutex_);
        repeated = players_.contains(pid);
    }

    if (repeated) {
        // TODO: Handle login repeated
    }

    auto plr = factory_.create();
    if (plr == nullptr) {
        // TODO: Handle nullptr
        return;
    }

    auto ctx = uranus::actor::MakeActorContext<PlayerRouter>(getWorld().getWorkerIOContext());
    ctx->getRouter().setGameWorld(&getWorld());
    ctx->attr().set("CONNECTION_KEY", key);

    // Set up actor to context
    ctx->setActor(std::move(plr));

    std::unique_lock lock(mutex_);
    players_.insert_or_assign(pid, ctx);
}

void PlayerManager::start() {
    factory_.initial();
}

void PlayerManager::stop() {
}
