#include "ServiceContext.h"
#include "ServiceManager.h"
#include "player/PlayerManager.h"
#include "player/PlayerContext.h"
#include "gateway/Gateway.h"
#include "gateway/ClientConnection.h"
#include "GameWorld.h"

#include <actor/BaseService.h>

namespace uranus {

    using actor::Package;
    using actor::Envelope;

    ServiceContext::ServiceContext(asio::io_context &ctx, ActorHandle &&actor)
        : BaseActorContext(ctx, std::move(actor)),
          manager_(nullptr) {
    }

    ServiceContext::~ServiceContext() {
    }

    // BaseService *ServiceContext::getService() const {
    //     if (auto *act = getActor()) {
    //         return dynamic_cast<BaseService *>(act);
    //     }
    //     return nullptr;
    // }

    void ServiceContext::send(const int ty, const int64_t target, PackageHandle &&pkg) {
        if ((ty & Package::kToService) != 0) {
            if (target == getId())
                return;

            if (const auto dest = manager_->find(target)) {
                Envelope envelope;

                envelope.type = (Package::kFromService | ty);
                envelope.source = getId();
                envelope.package = std::move(pkg);

                dest->pushEnvelope(std::move(envelope));
            }
        } else if ((ty & Package::kToPlayer) != 0) {
            if (const auto *playerManager = GetModule(PlayerManager)) {
                if (const auto plr = playerManager->find(target)) {
                    Envelope envelope;

                    envelope.type = (Package::kFromService | ty);
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

    ServerModule *ServiceContext::getModule(const std::string &name) const {
        if (manager_ && manager_->getModuleName() == name)
            return manager_;

        if (const auto *world = getWorld()) {
            return world->getModule(name);
        }

        return nullptr;
    }

    // std::map<std::string, uint32_t> ServiceContext::getServiceList() const {
    //     if (manager_) {
    //         return manager_->getServiceList();
    //     }
    //     return {};
    // }

    void ServiceContext::sendRequest(const int ty, const int64_t sess, const int64_t target, PackageHandle &&pkg) {
        if ((ty & Package::kToService) != 0) {
            if (target == getId())
                return;

            if (const auto dest = manager_->find(target)) {
                Envelope envelope;

                envelope.type = (Package::kFromService | ty);
                envelope.source = getId();
                envelope.session = sess;
                envelope.package = std::move(pkg);

                dest->pushEnvelope(std::move(envelope));
                return;
            }
        }

        if ((ty & Package::kToPlayer) != 0) {
            if (auto *mgr = GetModule(PlayerManager)) {
                if (const auto plr = mgr->find(target)) {
                    Envelope envelope;

                    envelope.type = (Package::kFromService | ty);
                    envelope.source = getId();
                    envelope.session = sess;
                    envelope.package = std::move(pkg);

                    plr->pushEnvelope(std::move(envelope));
                    return;
                }
            }
        }
    }

    void ServiceContext::sendResponse(const int ty, const int64_t sess, const int64_t target, PackageHandle &&pkg) {
        if ((ty & Package::kToService) != 0) {
            if (target == getId())
                return;

            if (const auto dest = manager_->find(target)) {
                Envelope envelope;

                envelope.type = (Package::kFromService | ty);
                envelope.source = getId();
                envelope.session = sess;
                envelope.package = std::move(pkg);

                dest->pushEnvelope(std::move(envelope));
            }
        }

        if ((ty & Package::kToPlayer) != 0) {
            if (auto *mgr = GetModule(PlayerManager)) {
                if (const auto plr = mgr->find(target)) {
                    Envelope envelope;

                    envelope.type = (Package::kFromService | ty);
                    envelope.source = getId();
                    envelope.session = sess;
                    envelope.package = std::move(pkg);

                    plr->pushEnvelope(std::move(envelope));
                }
            }
        }
    }

    void ServiceContext::setServiceManager(ServiceManager *mgr) {
        manager_ = mgr;
    }
}
