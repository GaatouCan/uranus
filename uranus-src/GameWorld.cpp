#include "GameWorld.h"

namespace uranus {
    GameWorld::GameWorld()
        : guard_(asio::make_work_guard(ctx_)){
    }

    GameWorld::~GameWorld() {
    }
}
