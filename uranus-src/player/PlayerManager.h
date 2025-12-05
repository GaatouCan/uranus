#pragma once

#include <base/ServerModule.h>


using uranus::ServerModule;
using uranus::ServerBootstrap;

class GameWorld;

class PlayerManager final : public ServerModule {

public:
    explicit PlayerManager(GameWorld &world);
    ~PlayerManager() override;

    constexpr const char *getModuleName() override {
        return "PlayerManager";
    }

    void start() override;
    void stop() override;
};
