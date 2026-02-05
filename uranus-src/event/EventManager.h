#pragma once

#include <actor/ServerModule.h>
#include <actor/Envelope.h>

#include <shared_mutex>
#include <unordered_map>
#include <set>


namespace uranus {

    using actor::ServerModule;
    using actor::DataAssetHandle;

    class GameWorld;

    class EventManager final : public ServerModule {

        struct ListenerSet {
            std::set<int64_t> services;
            std::set<int64_t> players;
        };

        using ListenerMap = std::unordered_map<int64_t, ListenerSet>;

    public:
        explicit EventManager(GameWorld &world);
        ~EventManager() override;

        DISABLE_COPY_MOVE(EventManager)
        SERVER_MODULE_NAME(EventManager)

        void start() override;
        void stop() override;

        void listenEvent(bool is_player, int64_t id, int64_t evt, bool cancel);
        void dispatchEvent(int64_t evt, DataAssetHandle &&data);

    private:
        GameWorld &world_;

        mutable std::shared_mutex mutex_;
        ListenerMap listeners_;
    };
}
