#include "GameWorld.h"
#include "ConfigModule.h"
#include "gateway/Gateway.h"
#include "player/PlayerManager.h"

using uranus::config::ConfigModule;

GameWorld::GameWorld() {
}

GameWorld::~GameWorld() {
}

void GameWorld::Start() {
    const auto *module = GetModule<ConfigModule>();
    const auto &config = module->GetServerConfig();

    io_num_ = config["server"]["network"]["worker"].as<int>();
    worker_num_ = config["server"]["actor"]["worker"].as<int>();

    GameServer::Start();
}


int main(int argc, char **argv) {
    auto *world = new GameWorld();

    world->CreateModule<ConfigModule>();
    world->CreateModule<PlayerManager>();
    world->CreateModule<Gateway>();

    world->Start();
    world->Stop();

    delete world;
}
