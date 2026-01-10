#include "PlayerContext.h"
#include "PlayerManager.h"

#include "GameWorld.h"
#include "gateway/ClientConnection.h"
#include "gateway/Gateway.h"
#include "service/ServiceManager.h"
#include "service/ServiceContext.h"

namespace uranus {

    using actor::Package;
    using actor::Envelope;

    PlayerContext::PlayerContext(asio::io_context &ctx, ActorHandle &&actor)
        : BaseActorContext(ctx, std::move(actor)),
          manager_(nullptr) {
    }

    PlayerContext::~PlayerContext() {
    }

    void PlayerContext::send(const int ty, const int64_t target, PackageHandle &&pkg) {
        const auto pid = getPlayerId();
        if (pid < 0)
            return;

        if ((ty & Package::kToService) != 0) {
            if (const auto *serviceMgr = GetModule(ServiceManager)) {
                if (const auto ser = serviceMgr->find(target)) {
                    Envelope envelope;

                    envelope.type = (Package::kFromPlayer | ty);
                    envelope.source = pid;
                    envelope.package = std::move(pkg);

                    ser->pushEnvelope(std::move(envelope));
                }
            }
        } else if ((ty & Package::kToClient) != 0) {
            if (const auto *gateway = GetModule(Gateway)) {
                if (const auto client = gateway->find(pid)) {
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

    void PlayerContext::setPlayerId(const int64_t pid) {
        attr().set("PLAYER_ID", pid);
    }

    int64_t PlayerContext::getPlayerId() const {
        if (const auto op = attr().get<int64_t>("PLAYER_ID"); op.has_value()) {
            return op.value();
        }
        return -1;
    }

    // std::map<std::string, uint32_t> PlayerContext::getServiceList() const {
    //     if (const auto *serviceMgr = GetModule(ServiceManager)) {
    //         return serviceMgr->getServiceList();
    //     }
    //     return {};
    // }

    void PlayerContext::sendRequest(const int ty, const int64_t sess, const int64_t target, PackageHandle &&pkg) {
        const auto pid = getPlayerId();
        if (pid < 0)
            return;

        if ((ty & Package::kToService) != 0) {
            if (const auto *mgr = GetModule(ServiceManager)) {
                if (const auto ctx = mgr->find(target)) {
                    Envelope envelope;

                    envelope.type = (Package::kFromPlayer | ty);
                    envelope.source = pid;
                    envelope.session = sess;
                    envelope.package = std::move(pkg);

                    ctx->pushEnvelope(std::move(envelope));
                }
            }
        }
    }

    void PlayerContext::sendResponse(const int ty, const int64_t sess, const int64_t target, PackageHandle &&pkg) {
        const auto pid = getPlayerId();
        if (pid < 0)
            return;

        if ((ty & Package::kToService) != 0) {
            if (const auto *mgr = GetModule(ServiceManager)) {
                if (const auto ctx = mgr->find(target)) {
                    Envelope envelope;

                    envelope.type = (Package::kFromPlayer | ty);
                    envelope.source = pid;
                    envelope.session = sess;
                    envelope.package = std::move(pkg);

                    ctx->pushEnvelope(std::move(envelope));
                }
            }
        }
    }


    void PlayerContext::setPlayerManager(PlayerManager *mgr) {
        manager_ = mgr;
    }
} // uranus
