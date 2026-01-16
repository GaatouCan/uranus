#pragma once

#include <tuple>
#include <base/Singleton.h>
#include <base/SharedLibrary.h>

namespace uranus {

    namespace actor {
        class BasePlayer;
    }

    using actor::BasePlayer;

    class PlayerFactory final : public Singleton<PlayerFactory> {

        friend class Singleton;

        using PlayerCreator = BasePlayer* (*)();
        using PlayerDeleter = void (*)(BasePlayer *);

        PlayerFactory();
        ~PlayerFactory() override;

    public:
        using InstanceResult = std::tuple<BasePlayer *, std::filesystem::path>;

        void initial();

        [[nodiscard]] InstanceResult create() const;
        void destroy(BasePlayer *plr);

        // [[nodiscard]] const SharedLibrary &getPlayerLibrary() const;

    private:
        SharedLibrary lib_;

        PlayerCreator creator_;
        PlayerDeleter deleter_;
    };
}