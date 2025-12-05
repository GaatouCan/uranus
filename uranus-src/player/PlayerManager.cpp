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

    // FIXME: Create player instance by factory

    auto ctx = uranus::actor::MakeActorContext<PlayerRouter>(getWorld().getWorkerIOContext());
    ctx->getRouter().setGameWorld(&getWorld());
    ctx->attr().set("CONNECTION_KEY", key);

    // TODO: Set up player to context

    std::unique_lock lock(mutex_);
    players_.insert_or_assign(pid, ctx);
}

void PlayerManager::start() {
}

void PlayerManager::stop() {
}
