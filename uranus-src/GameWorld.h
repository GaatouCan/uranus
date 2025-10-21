#pragma once

#include <GameServer.h>

using uranus::GameServer;

class GameWorld : public GameServer {

public:
    GameWorld();
    ~GameWorld() override;

    void Start() override;
};
