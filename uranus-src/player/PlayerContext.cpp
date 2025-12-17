#include "PlayerContext.h"
#include "PlayerManager.h"

#include "GameWorld.h"
#include "gateway/ActorConnection.h"
#include "gateway/Gateway.h"

namespace uranus {

    using actor::Package;

    PlayerContext::PlayerContext(asio::io_context &ctx)
        : ActorContext(ctx),
          manager_(nullptr) {
    }

    PlayerContext::~PlayerContext() {
    }

    void PlayerContext::send(int ty, uint32_t target, PackageHandle &&pkg) {
        if ((ty & Package::kToService) != 0) {
            // TODO
        } else if ((ty & Package::kToClient) != 0) {
            if (const auto *gateway = GetModule(Gateway)) {
                if (const auto client = gateway->findByPlayerID(getId())) {
                    client->send(std::move(pkg));
                    return;
                }
            }
        }
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
