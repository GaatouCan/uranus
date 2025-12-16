#pragma once

#include <actor/ServerModule.h>

#include <shared_mutex>
#include <unordered_map>

namespace uranus {

    using actor::ServerModule;
    using std::unordered_map;
    using std::shared_ptr;
    using std::shared_mutex;
    using std::unique_lock;
    using std::shared_lock;

    class GameWorld;
    class PlayerContext;

    class PlayerManager final : public ServerModule {

    public:
        explicit PlayerManager(GameWorld &world);
        ~PlayerManager() override;

        SERVER_MODULE_NAME(PlayerManager)

        [[nodiscard]] GameWorld &getWorld() const;

        void start() override;
        void stop() override;

        void onPlayerLogin(uint32_t pid);

        [[nodiscard]] shared_ptr<PlayerContext> find(uint32_t pid) const;
        void remove(uint32_t pid);

    private:
        GameWorld &world_;

        mutable shared_mutex mutex_;
        unordered_map<uint32_t, shared_ptr<PlayerContext>> players_;
    };
} // uranus
