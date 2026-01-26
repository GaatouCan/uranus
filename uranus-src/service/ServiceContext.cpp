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

    BaseService *ServiceContext::getService() const {
        return dynamic_cast<BaseService *>(getActor());
    }

    void ServiceContext::send(const int ty, const int64_t target, PackageHandle &&pkg) {
        const auto sid = getServiceId();
        if (sid < 0)
            return;

        // 发送至其它Service
        if ((ty & Envelope::kToService) != 0) {
            if (target == sid)
                return;

            if (const auto dest = manager_->find(target)) {
                Envelope evl((Envelope::kFromService | ty), sid, std::move(pkg));
                dest->pushEnvelope(std::move(evl));
            }
        }
        // 发送至Player
        else if ((ty & Envelope::kToPlayer) != 0) {
            if (const auto *mgr = GET_MODULE(getWorld(), PlayerManager)) {
                if (const auto plr = mgr->find(target)) {
                    Envelope evl((Envelope::kFromService | ty), sid, std::move(pkg));
                    plr->pushEnvelope(std::move(evl));
                }
            }
        }
        // 直接发送给客户端
        else if ((ty & Envelope::kToClient) != 0) {
            if (const auto *gateway = GET_MODULE(getWorld(), Gateway)) {
                if (const auto client = gateway->find(target)) {
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

    void ServiceContext::setServiceId(int64_t sid) {
        attr().set("SERVICE_ID", sid);
    }

    int64_t ServiceContext::getServiceId() const {
        if (const auto op = attr().get<int>("SERVICE_ID"); op.has_value()) {
            return op.value();
        }
        return -1;
    }

    void ServiceContext::sendRequest(const int ty, const int64_t sess, const int64_t target, PackageHandle &&pkg) {
        const auto sid = getServiceId();
        if (sid < 0)
            return;

        if ((ty & Envelope::kToService) != 0) {
            if (target == sid)
                return;

            if (const auto dest = manager_->find(target)) {
                Envelope evl((Envelope::kFromService | ty), sid, sess, std::move(pkg));
                dest->pushEnvelope(std::move(evl));
                return;
            }
        }

        if ((ty & Envelope::kToPlayer) != 0) {
            if (const auto *mgr = GET_MODULE(getWorld(), PlayerManager)) {
                if (const auto plr = mgr->find(target)) {
                    Envelope evl((Envelope::kFromService | ty), sid, sess, std::move(pkg));
                    plr->pushEnvelope(std::move(evl));
                    return;
                }
            }
        }
    }

    void ServiceContext::sendResponse(const int ty, const int64_t sess, const int64_t target, PackageHandle &&pkg) {
        const auto sid = getServiceId();
        if (sid < 0)
            return;

        if ((ty & Envelope::kToService) != 0) {
            if (target == sid)
                return;

            if (const auto dest = manager_->find(target)) {
                Envelope evl((Envelope::kFromService | ty), sid, sess, std::move(pkg));
                dest->pushEnvelope(std::move(evl));
            }
        }

        if ((ty & Envelope::kToPlayer) != 0) {
            if (const auto *mgr = GET_MODULE(getWorld(), PlayerManager)) {
                if (const auto plr = mgr->find(target)) {
                    Envelope evl((Envelope::kFromService | ty), sid, sess, std::move(pkg));
                    plr->pushEnvelope(std::move(evl));
                }
            }
        }
    }

    void ServiceContext::dispatchEvent(const int ty, const int64_t target, int64_t evt, DataAssetHandle &&data) {
        const auto sid = getServiceId();
        if (sid < 0)
            return;

        if ((ty & Envelope::kEvent) == 0)
            return;

        if ((ty & Envelope::kToService) != 0) {
            if (target == sid)
                return;

            if (const auto dest = manager_->find(target)) {
                Envelope evl((Envelope::kFromService | ty), sid, evt, std::move(data));
                dest->pushEnvelope(std::move(evl));
                return;
            }
        }

        if ((ty & Envelope::kToPlayer) != 0) {
            if (const auto *mgr = GET_MODULE(getWorld(), PlayerManager)) {
                if (const auto plr = mgr->find(target)) {
                    Envelope evl((Envelope::kFromService | ty), sid, evt, std::move(data));
                    plr->pushEnvelope(std::move(evl));
                    return;
                }
            }
        }
    }

    void ServiceContext::setServiceManager(ServiceManager *mgr) {
        manager_ = mgr;
    }
}
