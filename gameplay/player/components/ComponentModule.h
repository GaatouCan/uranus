#pragma once

#include "PlayerComponent.h"


namespace gameplay {

    class GamePlayer;

    class ComponentModule final {

        friend class GamePlayer;

        explicit ComponentModule(GamePlayer &plr);

    public:
        ComponentModule()= delete;
        ~ComponentModule();

        DISABLE_COPY_MOVE(ComponentModule)

        [[nodiscard]] GamePlayer &getPlayer() const;

    private:
        GamePlayer &owner_;
    };
}