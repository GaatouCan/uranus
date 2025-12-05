#include "GameWorld.h"

#include <ranges>

GameWorld::GameWorld() {
}

GameWorld::~GameWorld() {
}

void GameWorld::run() {
}

void GameWorld::terminate() {
    for (const auto &it: ordered_ | std::views::reverse) {
        it->stop();
    }
}
