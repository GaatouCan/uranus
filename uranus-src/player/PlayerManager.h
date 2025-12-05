#pragma once

#include <base/ServerModule.h>


using uranus::ServerModule;
using uranus::ServerBootstrap;

class GameWorld;

class PlayerManager final : public ServerModule {

public:
    explicit PlayerManager(GameWorld &world);
    ~PlayerManager() override;

    void start() override;
    void stop() override;
};
