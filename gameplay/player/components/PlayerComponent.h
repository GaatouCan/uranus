#pragma once

#include <base/noncopy.h>
#include <memory>
#include <vector>

namespace uranus::database {
    class Entity;
}

namespace gameplay {

    using uranus::database::Entity;
    using std::shared_ptr;
    using std::vector;
    using EntityList = vector<shared_ptr<Entity>>;

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