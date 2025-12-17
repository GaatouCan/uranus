#pragma once

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
        void initial();

        [[nodiscard]] BasePlayer* create() const;
        void destroy(BasePlayer *plr);

    private:
        SharedLibrary lib_;

        PlayerCreator creator_;
        PlayerDeleter deleter_;
    };
}