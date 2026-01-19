#pragma once

#include <base/noncopy.h>
#include <cstdint>

namespace gameplay {

    class ComponentModule;
    class GamePlayer;

    class PlayerComponent {

    public:
        PlayerComponent() = delete;

        explicit PlayerComponent(ComponentModule &module);
        virtual ~PlayerComponent();

        DISABLE_COPY_MOVE(PlayerComponent)

        [[nodiscard]] virtual const char *getComponentName() const = 0;

        [[nodiscard]] ComponentModule &getComponentModule() const;
        [[nodiscard]] GamePlayer &getPlayer() const;
        [[nodiscard]] int64_t getPlayerId() const;

        virtual void onLogin();
        virtual void onLogout();

    private:
        ComponentModule &module_;
    };
}