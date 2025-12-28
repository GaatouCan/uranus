#include "PlayerManager.h"
#include "PlayerContext.h"
#include "GameWorld.h"
#include "factory/PlayerFactory.h"

#include <actor/BasePlayer.h>

namespace uranus {

    using actor::BaseActor;

    PlayerManager::PlayerManager(GameWorld &world)
        : world_(world) {
    }

    PlayerManager::~PlayerManager() {
        players_.clear();
    }

    GameWorld &PlayerManager::getWorld() const {
        return world_;
    }

    void PlayerManager::start() {
        // Initial the player manager
        PlayerFactory::instance().initial();
    }

    void PlayerManager::stop() {
    }

    void PlayerManager::onPlayerLogin(const uint32_t pid) {
        if (!world_.isRunning())
            return;

        auto *plr = PlayerFactory::instance().create();

        if (!plr)
            return;

        auto ctx = std::make_shared<PlayerContext>(world_.getWorkerIOContext());

        shared_ptr<PlayerContext> old;

        {
            unique_lock lock(mutex_);

            // If the same player id actor has exists
            if (const auto it = players_.find(pid); it != players_.end()) {
                old = it->second;
                players_.erase(it);
            }

            players_.insert_or_assign(pid, ctx);
        }

        // Stop the old one
        if (old) {
            old->terminate();
        }

        ctx->setId(pid);
        ctx->setPlayerManager(this);
        ctx->setUpActor({plr, [](BaseActor *ptr) {
            if (!ptr)
                return;

            if (auto *temp = dynamic_cast<BasePlayer *>(ptr)) {
                PlayerFactory::instance().destroy(temp);
                return;
            }

            delete ptr;
        }});

        ctx->run();
    }

    shared_ptr<PlayerContext> PlayerManager::find(const uint32_t pid) const {
        if (!world_.isRunning())
            return nullptr;

        shared_lock lock(mutex_);
        const auto it = players_.find(pid);
        return it != players_.end() ? it->second : nullptr;
    }

    void PlayerManager::remove(const uint32_t pid) {
        if (!world_.isRunning())
            return;

        shared_ptr<PlayerContext> ctx;

        {
            unique_lock lock(mutex_);

            if (const auto it = players_.find(pid); it != players_.end()) {
                ctx = it->second;
                players_.erase(it);
            }
        }

        if (ctx) {
            ctx->terminate();
        }
    }
} // uranus
