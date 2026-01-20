#pragma once

#include <vector>
#include <map>
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>

#pragma region Components Header

#include "components/appear/AppearanceComponent.h"

#pragma endregion

namespace gameplay {

    using uranus::database::Entity;
    using std::shared_ptr;
    using std::map;
    using std::vector;
    using std::function;
    using std::unordered_map;

    using EntitiesMap = map<std::string, EntityList>;

    class GamePlayer;

    class ComponentModule final {

        friend class GamePlayer;

        using SerializeFunc = function<void()>;
        using DeserializeFunc = function<void(const EntityList &)>;

        struct RegisterData {
            std::string     table;
            SerializeFunc   ser;
            DeserializeFunc deser;
        };

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
        void registerComponent(PlayerComponent *comp, const vector<RegisterData> &list = {});

    private:
        GamePlayer &owner_;

        vector<PlayerComponent *> components_;

        unordered_map<std::string, DeserializeFunc> deserFuncs_;

#pragma region
        AppearanceComponent appearance_;
#pragma endregion
    };
}