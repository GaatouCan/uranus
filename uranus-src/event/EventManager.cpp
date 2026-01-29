#include "EventManager.h"

#include <ranges>

#include "GameWorld.h"

namespace uranus {
    EventManager::EventManager(GameWorld &world)
        : world_(world) {
    }

    EventManager::~EventManager() {
    }

    void EventManager::start() {
    }

    void EventManager::stop() {
    }

    void EventManager::listenEvent(const bool is_player, const int64_t id, const int evt, const bool cancel) {
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
            for (auto &[ser, plr] : listeners_ | std::views::values) {
                if (is_player) {
                    plr.erase(id);
                } else {
                    ser.erase(id);
                }
            }
        }
    }
}
