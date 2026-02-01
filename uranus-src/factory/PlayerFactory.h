#pragma once

#include <base/SharedLibrary.h>
#include <base/Singleton.h>
#include <tuple>
#include <atomic>

namespace uranus {

    namespace actor {
        class BasePlayer;
    }

    using actor::BasePlayer;
    using std::atomic_uint32_t;

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
        SharedLibrary lib_;

        PlayerCreator creator_;
        PlayerDeleter deleter_;

        atomic_uint32_t count_;
    };
}