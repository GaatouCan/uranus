#pragma once

#include "PlayerRouter.h"

#include <base/ServerModule.h>

#include <unordered_map>
#include <shared_mutex>
#include <memory>


using uranus::ServerModule;
using uranus::ServerBootstrap;

class GameWorld;

class PlayerManager final : public ServerModule {

public:
    using PlayerContextPointer = std::shared_ptr<uranus::actor::detail::ActorContextImpl<PlayerRouter>>;

    explicit PlayerManager(GameWorld &world);
    ~PlayerManager() override;

    constexpr const char *getModuleName() override {
        return "PlayerManager";
    }

    [[nodiscard]] GameWorld &getWorld() const;

    void start() override;
    void stop() override;

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<uint32_t, PlayerContextPointer> players_;
};
