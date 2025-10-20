#pragma once

#include "ServerModule.h"


using uranus::ServerModule;
using uranus::GameServer;

class PlayerManager final : public ServerModule {

public:
    explicit PlayerManager(GameServer *ser);
    ~PlayerManager() override;

    [[nodiscard]] constexpr const char *GetModuleName() const override {
        return "PlayerManager";
    }
};
