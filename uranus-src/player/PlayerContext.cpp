#include "PlayerContext.h"
#include "PlayerManager.h"

#include "GameWorld.h"
#include "gateway/ClientConnection.h"
#include "gateway/Gateway.h"
#include "service/ServiceManager.h"
#include "service/ServiceContext.h"
#include "event/EventManager.h"

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

        // 发送至Service
        if ((ty & Package::kToService) != 0) {
            if (const auto *mgr = GET_MODULE(getWorld(), ServiceManager)) {
                if (const auto ser = mgr->find(target)) {
                    auto evl = Envelope::makePackage((Package::kFromPlayer | ty), pid, std::move(pkg));
                    ser->pushEnvelope(std::move(evl));
                }
            }
        }
        // 发送给客户端
        else if ((ty & Package::kToClient) != 0) {
            if (const auto *gateway = GET_MODULE(getWorld(), Gateway)) {
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

    ActorMap PlayerContext::getActorMap(const std::string &type) const {
        if (type == "service") {
            if (const auto *mgr = GET_MODULE(getWorld(), ServiceManager)) {
                return mgr->getServiceMap();
            }
        } else if (type == "player") {
            // TODO:
        }

        return {};
    }

    int64_t PlayerContext::queryActorId(const std::string &type, const std::string &name) const {
        if (type == "service") {
            if (const auto *mgr = GET_MODULE(getWorld(), ServiceManager)) {
                return mgr->queryServiceId(name);
            }
        } else if (type == "player") {
            // TODO
        }
        return -1;
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

    void PlayerContext::sendRequest(const int ty, const int64_t sess, const int64_t target, PackageHandle &&pkg) {
        const auto pid = getPlayerId();
        if (pid < 0)
            return;

        // 只能向Service异步请求
        if ((ty & Package::kToService) != 0) {
            if (const auto *mgr = GET_MODULE(getWorld(), ServiceManager)) {
                if (const auto ctx = mgr->find(target)) {
                    auto evl = Envelope::makeRequest((ty | Package::kFromPlayer), pid, sess, std::move(pkg));
                    ctx->pushEnvelope(std::move(evl));
                }
            }
        }
    }

    void PlayerContext::sendResponse(const int ty, const int64_t sess, const int64_t target, PackageHandle &&pkg) {
        const auto pid = getPlayerId();
        if (pid < 0)
            return;

        // 只接受来自Service的异步请求
        if ((ty & Package::kToService) != 0) {
            if (const auto *mgr = GET_MODULE(getWorld(), ServiceManager)) {
                if (const auto ctx = mgr->find(target)) {
                    auto evl = Envelope::makeResponse((ty | Package::kFromPlayer), pid, sess, std::move(pkg));
                    ctx->pushEnvelope(std::move(evl));
                }
            }
        }
    }

    void PlayerContext::cleanUp() {
        super::cleanUp();

        if (auto *mgr = GET_MODULE(getWorld(), EventManager)) {
            mgr->listenEvent(true, getPlayerId(), -1, true);
        }
    }

    void PlayerContext::dispatch(const int64_t evt, DataAssetHandle &&data) {
        // const auto pid = getPlayerId();
        // if (pid < 0)
        //     return;

        if (auto *mgr = GET_MODULE(getWorld(), EventManager)) {
            mgr->dispatchEvent(evt, std::move(data));
        }
    }

    void PlayerContext::listen(const int64_t evt, const bool cancel) {
        const auto pid = getPlayerId();
        if (pid < 0)
            return;

        if (auto *mgr = GET_MODULE(getWorld(), EventManager)) {
            mgr->listenEvent(true, pid, evt, cancel);
        }
    }

    void PlayerContext::setPlayerManager(PlayerManager *mgr) {
        manager_ = mgr;
    }
} // uranus
