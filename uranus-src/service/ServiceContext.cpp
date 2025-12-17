#include "ServiceContext.h"
#include "ServiceManager.h"
#include "player/PlayerManager.h"
#include "player/PlayerContext.h"
#include "gateway/Gateway.h"
#include "gateway/ActorConnection.h"
#include "GameWorld.h"

#include <actor/BaseService.h>

namespace uranus {

    using actor::Package;
    using actor::Envelope;

    ServiceContext::ServiceContext(asio::io_context &ctx)
        : ActorContext(ctx),
          manager_(nullptr) {
    }

    ServiceContext::~ServiceContext() {
    }

    BaseService *ServiceContext::getService() const {
        if (auto *act = getActor()) {
            return dynamic_cast<BaseService *>(act);
        }
        return nullptr;
    }

    void ServiceContext::send(const int ty, const uint32_t target, PackageHandle &&pkg) {
        if ((ty & Package::kToService) != 0) {
            if (target == getId())
                return;

            if (const auto dest = manager_->find(target)) {
                Envelope envelope;

                envelope.type = (Package::kFromService | Package::kToService);
                envelope.source = getId();
                envelope.package = std::move(pkg);

                dest->pushEnvelope(std::move(envelope));
            }
        } else if ((ty & Package::kToPlayer) != 0) {
            if (const auto *playerManager = GetModule(PlayerManager)) {
                if (const auto plr = playerManager->find(target)) {
                    Envelope envelope;

                    envelope.type = (Package::kFromService | Package::kToPlayer);
                    envelope.source = getId();
                    envelope.package = std::move(pkg);

                    plr->pushEnvelope(std::move(envelope));
                }
            }
        } else if ((ty & Package::kToClient) != 0) {
            if (const auto *gateway = GetModule(Gateway)) {
                if (const auto client = gateway->findByPlayerID(target)) {
                    client->send(std::move(pkg));
                }
            }
        }
    }

    ServiceManager *ServiceContext::getServiceManager() const {
        return manager_;
    }

    GameWorld *ServiceContext::getWorld() const {
        if (manager_) {
            return &manager_->getWorld();
        }
        return nullptr;
    }

    void ServiceContext::setServiceManager(ServiceManager *mgr) {
        manager_ = mgr;
    }
}
