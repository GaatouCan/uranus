#include "PlayerContext.h"
#include "PlayerManager.h"

#include "GameWorld.h"
#include "gateway/ActorConnection.h"
#include "gateway/Gateway.h"
#include "service/ServiceManager.h"
#include "service/ServiceContext.h"

namespace uranus {

    using actor::Package;
    using actor::Envelope;

    PlayerContext::PlayerContext(asio::io_context &ctx)
        : ActorContext(ctx),
          manager_(nullptr) {
    }

    PlayerContext::~PlayerContext() {
    }

    void PlayerContext::send(const int ty, const uint32_t target, PackageHandle &&pkg) {
        if ((ty & Package::kToService) != 0) {
            if (const auto *serviceMgr = GetModule(ServiceManager)) {
                if (const auto ser = serviceMgr->find(target)) {
                    Envelope envelope;

                    envelope.type = (Package::kFromPlayer | Package::kToService);
                    envelope.source = getId();
                    envelope.package = std::move(pkg);

                    ser->pushEnvelope(std::move(envelope));
                }
            }
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

    ServerModule *PlayerContext::getModule(const std::string &name) const {
        if (manager_ && manager_->getModuleName() == name)
            return manager_;

        if (const auto *world = getWorld()) {
            return world->getModule(name);
        }

        return nullptr;
    }

    void PlayerContext::setPlayerManager(PlayerManager *mgr) {
        manager_ = mgr;
    }
} // uranus
