#include "PlayerManager.h"
#include "AbstractPlayer.h"
#include "PlayerContext.h"
#include "../GameWorld.h"
#include "../factory/PlayerFactory.h"

#include <ranges>
#include <spdlog/spdlog.h>


PlayerManager::PlayerManager(GameServer *ser)
    : ServerModule(ser) {
    factory_ = make_unique<PlayerFactory>();
}

PlayerManager::~PlayerManager() {
}

int PlayerManager::OnPlayerLogin(const int64_t pid) {
    if (!GetGameServer()->IsRunning())
        return 0;

    shared_ptr<PlayerContext> old;

    {
        unique_lock lock(mutex_);
        const auto iter = players_.find(pid);
        if (iter != players_.end()) {
            old = iter->second;
            players_.erase(iter);
        }
    }

    if (old != nullptr) {
        old->Stop();
    }

    auto plr = factory_->CreatePlayer();
    plr->SetPlayerID(pid);

    auto ctx = make_shared<PlayerContext>(dynamic_cast<GameWorld *>(GetGameServer()));
    ctx->SetUpPlayer(std::move(plr));

    if (const auto ret = ctx->Initial(nullptr); ret != 1) {
        SPDLOG_ERROR("Player[{}] - Failed to initial context, code: {}", pid, ret);
        ctx->Stop();
        return ret;
    }

    if (const auto ret = ctx->Start(); ret != 1) {
        SPDLOG_ERROR("Player[{}] - Failed to strat context, code: {}", pid, ret);
        ctx->Stop();
        return ret;
    }

    unique_lock lock{mutex_};
    players_.insert_or_assign(pid, ctx);

    SPDLOG_INFO("Player[{}] - Create player context success", pid);
    return 1;
}

shared_ptr<PlayerContext> PlayerManager::FindPlayer(const int64_t pid) const {
    shared_lock lock(mutex_);
    const auto iter = players_.find(pid);
    return iter == players_.end() ? nullptr : iter->second;
}

void PlayerManager::OnPlayerLogout(const int64_t pid) {
    if (!GetGameServer()->IsRunning())
        return;

    shared_ptr<PlayerContext> ctx;

    {
        unique_lock lock(mutex_);
        const auto iter = players_.find(pid);
        if (iter == players_.end())
            return;

        ctx = iter->second;
        players_.erase(iter);
    }

    if (ctx == nullptr)
        return;

    ctx->Stop();
}

void PlayerManager::Start() {

}

void PlayerManager::Stop() {
    for (const auto &plr : players_ | std::views::values) {
        plr->Stop();
    }
}
