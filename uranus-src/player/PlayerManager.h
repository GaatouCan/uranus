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

        DISABLE_COPY_MOVE(PlayerManager)
        SERVER_MODULE_NAME(PlayerManager)

        [[nodiscard]] GameWorld &getWorld() const;

        void start() override;
        void stop() override;

        void onPlayerLogin(int64_t pid);
        void onPlayerLogout(int64_t pid);

        [[nodiscard]] shared_ptr<PlayerContext> find(int64_t pid) const;

    private:
        GameWorld &world_;

        mutable shared_mutex mutex_;
        unordered_map<int64_t, shared_ptr<PlayerContext>> players_;
    };
} // uranus
