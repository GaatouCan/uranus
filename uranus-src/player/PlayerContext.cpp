#include "PlayerContext.h"

#include "PlayerManager.h"

namespace uranus {
    PlayerContext::PlayerContext(asio::io_context &ctx)
        : ActorContext(ctx),
          manager_(nullptr) {
    }

    PlayerContext::~PlayerContext() {
    }

    void PlayerContext::send(int ty, uint32_t target, actor::PackageHandle &&pkg) {
    }

    PlayerManager *PlayerContext::getPlayerManager() const {
        return manager_;
    }

    GameWorld *PlayerContext::getWorld() const {
        if (manager_) {
            return &manager_->getWorld();
        }
        return nullptr;
    }

    void PlayerContext::setPlayerManager(PlayerManager *mgr) {
        manager_ = mgr;
    }
} // uranus
