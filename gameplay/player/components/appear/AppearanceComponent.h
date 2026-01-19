#pragma once

#include "components/PlayerComponent.h"

namespace gameplay {

    class AppearanceComponent final : public PlayerComponent {

        using super = PlayerComponent;

    public:
        explicit AppearanceComponent(ComponentModule &module);
        ~AppearanceComponent() override;

        [[nodiscard]] constexpr const char *getComponentName() const override {
            return "Appearance";
        }

        void onLogin() override;

        void sendInfo() const;

    private:
        int curAvatar_;
        int curFrame_;
        int curBackground_;
    };
}
