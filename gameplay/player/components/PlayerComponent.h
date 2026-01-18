#pragma once

#include <base/noncopy.h>

namespace gameplay {

    class ComponentModule;

    class PlayerComponent {

    public:
        PlayerComponent() = delete;

        explicit PlayerComponent(ComponentModule &module);
        virtual ~PlayerComponent();

        DISABLE_COPY_MOVE(PlayerComponent)

        [[nodiscard]] virtual const char *getComponentName() const = 0;

        [[nodiscard]] ComponentModule &getComponentModule() const;

        virtual void onLogin();
        virtual void onLogout();

    private:
        ComponentModule &module_;
    };

}