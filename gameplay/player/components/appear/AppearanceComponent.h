#pragma once

#include "components/PlayerComponent.h"

#include <nlohmann/json.hpp>

namespace gameplay {

    class AppearanceComponent final : public PlayerComponent {

        using super = PlayerComponent;

    public:
        explicit AppearanceComponent(ComponentModule &module);
        ~AppearanceComponent() override;

        [[nodiscard]] constexpr const char *getComponentName() const override {
            return "Appearance";
        }

        void serialize_Appearance(nlohmann::json &data);
        void deserialize_Appearance(const EntityList &list);

        void onLogin() override;

        void sendInfo() const;

    private:
        int curAvatar_;
        int curFrame_;
        int curBackground_;
    };
}
