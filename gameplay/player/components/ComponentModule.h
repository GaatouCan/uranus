#pragma once

#include <vector>


#pragma region Components Header

#include "components/appear/AppearanceComponent.h"

#pragma endregion

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

        void onLogin() const;
        void onLogout() const;

#pragma region Getter
        AppearanceComponent &getAppearance() { return appearance_; }
#pragma endregion

    private:
        void registerComponent(PlayerComponent *comp);

    private:
        GamePlayer &owner_;

        std::vector<PlayerComponent *> components_;

#pragma region
        AppearanceComponent appearance_;
#pragma endregion
    };
}