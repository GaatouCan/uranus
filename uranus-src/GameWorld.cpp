#include "GameWorld.h"

namespace uranus {
    GameWorld::GameWorld()
        : guard_(asio::make_work_guard(ctx_)) {
    }

    GameWorld::~GameWorld() {
    }

    void GameWorld::run() {
    }

    void GameWorld::terminate() {
    }

    bool GameWorld::isRunning() const {
        return !ctx_.stopped();
    }

    asio::io_context &GameWorld::getIOContext() {
        return ctx_;
    }

    asio::io_context &GameWorld::getWorkerIOContext() {
        return pool_.getIOContext();
    }
}
