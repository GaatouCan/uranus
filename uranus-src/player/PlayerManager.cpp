#include "PlayerManager.h"

#include "AbstractPlayer.h"
#include "PlayerContext.h"
#include "../GameWorld.h"
#include "../factory/PlayerFactory.h"

PlayerManager::PlayerManager(GameServer *ser)
    : ServerModule(ser) {

    factory_ = make_unique<PlayerFactory>();
}

PlayerManager::~PlayerManager() {
}

void PlayerManager::OnPlayerLogin(const int64_t pid) {
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
        // TODO: Handle Old
    }

    auto plr = factory_->CreatePlayer();
    plr->SetPlayerID(pid);

    auto ctx = make_shared<PlayerContext>(dynamic_cast<GameWorld *>(GetGameServer()));
    ctx->SetUpPlayer(std::move(plr));

    if (const auto ret = ctx->Initial(nullptr); ret != 1) {
        // TODO
        ctx->Stop();
        return;
    }

    if (const auto ret = ctx->Start(); ret != 1) {
        // TODO
        ctx->Stop();
        return;
    }

    unique_lock lock{mutex_};
    players_.insert_or_assign(pid, ctx);
}

shared_ptr<PlayerContext> PlayerManager::FindPlayer(const int64_t pid) const {
    shared_lock lock(mutex_);
    const auto iter = players_.find(pid);
    return iter == players_.end() ? nullptr : iter->second;
}

void PlayerManager::OnPlayerLogout(const int64_t pid) {
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
    factory_->Initial();
}
