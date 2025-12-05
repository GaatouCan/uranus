#include <spdlog/spdlog.h>

#include "GameWorld.h"
#include "gateway/Gateway.h"
#include "player/PlayerManager.h"

int main() {
    spdlog::info("Hello World!");

    auto *world = new GameWorld();

    // Create GameWorld Modules
    world->createModule<PlayerManager>();
    world->createModule<Gateway>();

    // Run the GameWorld
    world->run();
    world->terminate();

    delete world;

    return 0;
}
