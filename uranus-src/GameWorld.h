#pragma once

#include <base/ServerBootstrap.h>

using uranus::ServerBootstrap;

class GameWorld final : public ServerBootstrap {

public:
    GameWorld();
    ~GameWorld() override;

    void run() override;
    void terminate() override;
};
