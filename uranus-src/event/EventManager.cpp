#include "EventManager.h"
#include "GameWorld.h"
#include "player/PlayerManager.h"
#include "player/PlayerContext.h"
#include "service/ServiceManager.h"
#include "service/ServiceContext.h"

#include <ranges>
#include <spdlog/spdlog.h>

namespace uranus {

    using actor::Envelope;
    using std::make_unique;
    using std::unique_ptr;

    EventManager::EventManager(GameWorld &world)
        : world_(world) {
        SPDLOG_DEBUG("EventManager created");
    }

    EventManager::~EventManager() {
        SPDLOG_DEBUG("EventManager destroyed");
    }

    void EventManager::start() {
    }

    void EventManager::stop() {
    }

    void EventManager::listenEvent(
        const bool is_player,
        const int64_t id,
        const int64_t evt,
        const bool cancel
    ) {
        if (!world_.isRunning())
            return;

        std::unique_lock lock(mutex_);

        if (evt > 0) {
            auto &[services, players] = listeners_[evt];
            if (cancel) {
                if (is_player) {
                    players.erase(id);
                } else {
                    services.erase(id);
                }
            } else {
                if (is_player) {
                    players.insert(id);
                } else {
                    services.insert(id);
                }
            }
        } else {
            for (auto &[ser, plr]: listeners_ | std::views::values) {
                if (is_player) {
                    plr.erase(id);
                } else {
                    ser.erase(id);
                }
            }
        }
    }

    void EventManager::dispatchEvent(int64_t evt, DataAssetHandle &&data) {
        if (!world_.isRunning())
            return;

        if (evt < 0)
            return;

        // Dispatch event in the main thread
        asio::post(world_.getIOContext(), [this, evt, data = std::move(data)] {
            if (!world_.isRunning())
                return;

            ListenerSet set; {
                std::shared_lock lock(mutex_);
                const auto it = listeners_.find(evt);
                if (it == listeners_.end())
                    return;
                set = it->second;
            }

            if (const auto *mgr = GET_MODULE(&world_, ServiceManager)) {
                for (const auto &ctx: mgr->getServiceSet(set.services)) {
                    if (data != nullptr) {
                        auto evl = Envelope::makeDataAsset(evt, DataAssetHandle(data->clone()));
                        ctx->pushEnvelope(std::move(evl));
                    }
                }
            }

            if (const auto *mgr = GET_MODULE(&world_, PlayerManager)) {
                for (const auto &plr: mgr->getPlayerSet(set.players)) {
                    if (data != nullptr) {
                        auto evl = Envelope::makeDataAsset(evt, DataAssetHandle(data->clone()));
                        plr->pushEnvelope(std::move(evl));
                    }
                }
            }
        });
    }
}
