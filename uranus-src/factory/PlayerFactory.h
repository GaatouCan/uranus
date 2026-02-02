#pragma once

#include <base/SharedLibrary.h>
#include <base/Singleton.h>
#include <tuple>
#include <atomic>
#include <shared_mutex>

namespace uranus {

    namespace actor {
        class BasePlayer;
    }

    using actor::BasePlayer;
    using std::atomic_uint32_t;
    using std::shared_mutex;
    using std::shared_lock;
    using std::unique_lock;

    class PlayerFactory final : public Singleton<PlayerFactory> {

        friend class Singleton;

        using PlayerCreator = BasePlayer* (*)();
        using PlayerDeleter = void (*)(BasePlayer *);

        PlayerFactory();
        ~PlayerFactory() override;

    public:
        using InstanceResult = std::tuple<BasePlayer *, std::filesystem::path>;

        void initial();

        [[nodiscard]] InstanceResult create();
        void destroy(BasePlayer *plr);

        void release();
        void reload();

    private:
        mutable shared_mutex mutex_;
        SharedLibrary lib_;

        PlayerCreator creator_;
        PlayerDeleter deleter_;

        atomic_uint32_t count_;
    };
}

#define PLAYER_FACTORY PlayerFactory::instance()