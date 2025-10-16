#include "GameWorld.h"

GameWorld::GameWorld() {
}

GameWorld::~GameWorld() {
}


int main(int argc, char **argv) {
    auto *world = new GameWorld();

    world->Start();
    world->Stop();

    delete world;
}