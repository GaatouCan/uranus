#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

#pragma region Components Header

#include "components/appear/AppearanceComponent.h"

#pragma endregion

namespace gameplay {

    using uranus::database::Entity;
    using std::shared_ptr;
    using std::vector;
    using std::unordered_map;

    using EntitiesMap = unordered_map<std::string, EntityList>;

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
        void deserialize(const EntitiesMap& entities);

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