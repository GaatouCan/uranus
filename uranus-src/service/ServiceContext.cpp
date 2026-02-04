#include "ServiceContext.h"
#include "ServiceManager.h"
#include "player/PlayerManager.h"
#include "player/PlayerContext.h"
#include "gateway/Gateway.h"
#include "gateway/ClientConnection.h"
#include "event/EventManager.h"
#include "monitor/WorldMonitor.h"
#include "GameWorld.h"

#include <actor/BaseService.h>
#include <asio/bind_allocator.hpp>


namespace uranus {

    using actor::Package;
    using actor::Envelope;

    ServiceContext::ServiceContext(asio::any_io_executor ctx, ActorHandle &&actor)
        : BaseActorContext(std::move(ctx), std::move(actor)),
          manager_(nullptr) {
    }

    ServiceContext::~ServiceContext() {
    }

    BaseService *ServiceContext::getService() const {
        return dynamic_cast<BaseService *>(getActor());
    }

    void ServiceContext::send(const int ty, const int64_t target, PackageHandle &&pkg) {
        if (!isRunning())
            return;

        const auto sid = getServiceId();
        if (sid < 0)
            return;

        // 发送至其它Service
        if ((ty & Package::kToService) != 0) {
            if (target == sid)
                return;

            if (const auto dest = manager_->find(target)) {
                auto evl = Envelope::makePackage((Package::kFromService | ty), sid, std::move(pkg));
                dest->pushEnvelope(std::move(evl));
            }
        }
        // 发送至Player
        else if ((ty & Package::kToPlayer) != 0) {
            if (const auto *mgr = GET_MODULE(getWorld(), PlayerManager)) {
                if (const auto plr = mgr->find(target)) {
                    auto evl = Envelope::makePackage((Package::kFromService | ty), sid, std::move(pkg));
                    plr->pushEnvelope(std::move(evl));
                }
            }
        }
        // 直接发送给客户端
        else if ((ty & Package::kToClient) != 0) {
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

    ActorMap ServiceContext::getActorMap(const std::string &type) const {
        if (type == "service") {
            return manager_->getServiceMap();
        }

        if (type == "player") {
            // TODO
        }

        return {};
    }

    int64_t ServiceContext::queryActorId(const std::string &type, const std::string &name) const {
        if (type == "service") {
            return manager_->queryServiceId(name);
        }

        if (type == "player") {
            // TODO
        }

        return -1;
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
        if (!isRunning())
            return;

        const auto sid = getServiceId();
        if (sid < 0)
            return;

        if ((ty & Package::kToService) != 0) {
            if (target == sid)
                return;

            if (const auto dest = manager_->find(target)) {
                auto evl = Envelope::makeRequest((Package::kFromService | ty), sid, sess, std::move(pkg));
                dest->pushEnvelope(std::move(evl));
                return;
            }
        }

        if ((ty & Package::kToPlayer) != 0) {
            if (const auto *mgr = GET_MODULE(getWorld(), PlayerManager)) {
                if (const auto plr = mgr->find(target)) {
                    auto evl = Envelope::makeRequest((Package::kFromService | ty), sid, sess, std::move(pkg));
                    plr->pushEnvelope(std::move(evl));
                    return;
                }
            }
        }
    }

    void ServiceContext::sendResponse(const int ty, const int64_t sess, const int64_t target, PackageHandle &&pkg) {
        if (!isRunning())
            return;

        const auto sid = getServiceId();
        if (sid < 0)
            return;

        if ((ty & Package::kToService) != 0) {
            if (target == sid)
                return;

            if (const auto dest = manager_->find(target)) {
                auto evl = Envelope::makeResponse((Package::kFromService | ty), sid, sess, std::move(pkg));
                dest->pushEnvelope(std::move(evl));
            }
        }

        if ((ty & Package::kToPlayer) != 0) {
            if (const auto *mgr = GET_MODULE(getWorld(), PlayerManager)) {
                if (const auto plr = mgr->find(target)) {
                    auto evl = Envelope::makeResponse((Package::kFromService | ty), sid, sess, std::move(pkg));
                    plr->pushEnvelope(std::move(evl));
                }
            }
        }
    }

    bool ServiceContext::cleanUp() {
        if (!super::cleanUp())
            return false;

        if (auto *mgr = GET_MODULE(getWorld(), EventManager)) {
            mgr->listenEvent(false, getServiceId(), -1, true);
        }

        return true;
    }

    void ServiceContext::dispatch(int64_t evt, DataAssetHandle &&data) {
        if (!isRunning())
            return;

        if (auto *mgr = GET_MODULE(getWorld(), EventManager)) {
            mgr->dispatchEvent(evt, std::move(data));
        }
    }

    void ServiceContext::listen(const int64_t evt, const bool cancel) {
        if (isTerminated())
            return;

        const auto sid = getServiceId();
        if (sid < 0)
            return;

        if (auto *mgr = GET_MODULE(getWorld(), EventManager)) {
            mgr->listenEvent(false, sid, evt, cancel);
        }
    }

    void ServiceContext::createCommand(const std::string &cmd, DataAssetHandle &&data, CommandHandler &&handler) {
        auto kDispatchResult = [](CommandHandler &&h, DataAssetHandle &&res) mutable {
            const auto work = asio::make_work_guard(h);
            const auto alloc = asio::get_associated_allocator(h, asio::recycling_allocator<void>());

            asio::dispatch(
                work.get_executor(),
                asio::bind_allocator(
                    alloc,
                    [handler = std::move(h), result = std::move(res)]() mutable {
                        std::move(handler)(std::move(result));
                    }
                )
            );
        };

        if (isTerminated()) {
            kDispatchResult(std::move(handler), nullptr);
            return;
        }

        auto *monitor = GET_MODULE(getWorld(), WorldMonitor);
        if (monitor == nullptr) {
            kDispatchResult(std::move(handler), nullptr);
            return;
        }

        // TODO
        kDispatchResult(std::move(handler), nullptr);
    }

    void ServiceContext::setServiceManager(ServiceManager *mgr) {
        if (!isInitial())
            return;

        manager_ = mgr;
    }
}
