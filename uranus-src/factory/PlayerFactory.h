#pragma once

#include <base/noncopy.h>
#include <base/SharedLibrary.h>

namespace uranus {

    namespace actor {
        class BasePlayer;
    }

    using actor::BasePlayer;

    class PlayerFactory final {

        using PlayerCreator = BasePlayer* (*)();
        using PlayerDeleter = void (*)(BasePlayer *);

        PlayerFactory();

    public:
        ~PlayerFactory();

        DISABLE_COPY_MOVE(PlayerFactory)

        static PlayerFactory &instance();

        void initial();

        [[nodiscard]] BasePlayer* create() const;
        void destroy(BasePlayer *plr);

    private:
        SharedLibrary lib_;

        PlayerCreator creator_;
        PlayerDeleter deleter_;
    };
}