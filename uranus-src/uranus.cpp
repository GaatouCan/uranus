#include <spdlog/spdlog.h>

#include "GameWorld.h"
#include "gateway/Gateway.h"
#include "player/PlayerManager.h"
#include "service/ServiceManager.h"

using uranus::GameWorld;
using uranus::PlayerManager;
using uranus::ServiceManager;
using uranus::Gateway;


int main() {
    spdlog::info("Hello World!");

    auto *world = new GameWorld();

    world->pushModule(new PlayerManager(*world));
    world->pushModule(new ServiceManager(*world));
    world->pushModule(new Gateway(*world));

    world->run();
    world->terminate();

    delete world;

    return 0;
}
