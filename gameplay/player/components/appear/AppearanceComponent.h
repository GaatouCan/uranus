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

        void serialize_Appearance(nlohmann::json &data) const;
        void deserialize_Appearance(const nlohmann::json &data);

        void onLogin() override;

        void sendInfo() const;

        [[nodiscard]] int getCurrentAvatar() const      { return curAvatar_; }
        [[nodiscard]] int getCurrentFrame() const       { return curFrame_; }
        [[nodiscard]] int getCurrentBackground() const  { return curBackground_; }

    private:
        int curAvatar_;
        int curFrame_;
        int curBackground_;
    };
}
