#pragma once

#include <vector>
#include <unordered_map>

#pragma region Components Header

#include "components/appear/AppearanceComponent.h"

#pragma endregion

namespace gameplay {

    using std::vector;
    using std::unordered_map;

    class GamePlayer;

    class ComponentModule final {

        friend class GamePlayer;

        explicit ComponentModule(GamePlayer &plr);

    public:
        ComponentModule()= delete;
        ~ComponentModule();

        DISABLE_COPY_MOVE(ComponentModule)

        [[nodiscard]] GamePlayer &getPlayer() const;

        void serialize();
        void deserialize();

        void onLogin() const;
        void onLogout() const;

#pragma region Getter
        AppearanceComponent &getAppearance() { return appearance_; }
#pragma endregion

    private:
        void registerComponent(PlayerComponent *comp);

    private:
        GamePlayer &owner_;
        vector<PlayerComponent *> components_;

#pragma region
        AppearanceComponent appearance_;
#pragma endregion
    };
}