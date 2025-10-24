#pragma once

#include "ServerModule.h"

#include <memory>
#include <shared_mutex>
#include <unordered_map>


class PlayerFactory;
class PlayerContext;

using std::shared_ptr;
using std::unique_ptr;
using std::make_shared;
using std::make_unique;
using std::unordered_map;
using std::shared_mutex;
using std::shared_lock;
using std::unique_lock;
using uranus::ServerModule;
using uranus::GameServer;


class PlayerManager final : public ServerModule {

public:
    explicit PlayerManager(GameServer *ser);
    ~PlayerManager() override;

    [[nodiscard]] constexpr const char *GetModuleName() const override {
        return "PlayerManager";
    }

    int OnPlayerLogin(int64_t pid);
    void OnPlayerLogout(int64_t pid);

    [[nodiscard]] shared_ptr<PlayerContext> FindPlayer(int64_t pid) const;

    void Start() override;

    void Stop() override;

private:
    unique_ptr<PlayerFactory> factory_;

    mutable shared_mutex mutex_;
    unordered_map<int64_t, shared_ptr<PlayerContext>> players_;
};
