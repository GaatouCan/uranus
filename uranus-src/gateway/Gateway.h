#pragma once

#include <base/ServerModule.h>

using uranus::ServerModule;
using uranus::ServerBootstrap;

class GameWorld;

class Gateway final : public ServerModule {

public:
    explicit Gateway(ServerBootstrap &server);
    ~Gateway() override;

    [[nodiscard]] GameWorld &getWorld() const;

    void start() override;
    void stop() override;
};
