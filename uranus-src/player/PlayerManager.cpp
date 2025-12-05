#include "PlayerManager.h"
#include "../GameWorld.h"

PlayerManager::PlayerManager(GameWorld &world)
    : ServerModule(world) {
}

PlayerManager::~PlayerManager() {
}

GameWorld &PlayerManager::getWorld() const {
    return dynamic_cast<GameWorld &>(getServer());
}

void PlayerManager::createPlayer(uint32_t pid, const std::string &key) {
    // TODO: Create player instance

    auto ctx = uranus::actor::MakeActorContext<PlayerRouter>(getWorld().getWorkerIOContext());
    ctx->getRouter().setGameWorld(&getWorld());
    ctx->attr().set("CONNECTION_KEY", key);

    // TODO: Set up player to context
}

void PlayerManager::start() {
}

void PlayerManager::stop() {
}
