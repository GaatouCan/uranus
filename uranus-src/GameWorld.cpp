#include "GameWorld.h"

#include "gateway/Gateway.h"

GameWorld::GameWorld() {
}

GameWorld::~GameWorld() {
}


int main(int argc, char **argv) {
    auto *world = new GameWorld();

    world->CreateModule<Gateway>();

    world->Start();
    world->Stop();

    delete world;
}