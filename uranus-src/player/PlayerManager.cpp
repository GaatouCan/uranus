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

void PlayerManager::start() {
}

void PlayerManager::stop() {
}
