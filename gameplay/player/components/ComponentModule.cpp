#include "ComponentModule.h"

namespace gameplay {
    ComponentModule::ComponentModule(GamePlayer &plr)
        : owner_(plr),
#pragma region
          appearance_(*this)
#pragma endregion
    {
        registerComponent(&appearance_);
    }

    ComponentModule::~ComponentModule() {
    }

    GamePlayer &ComponentModule::getPlayer() const {
        return owner_;
    }

    void ComponentModule::onLogin() const {
        for (const auto &val : components_) {
            val->onLogin();
        }
    }

    void ComponentModule::onLogout() const {
        for (const auto &val : components_) {
            val->onLogout();
        }
    }

    void ComponentModule::registerComponent(PlayerComponent *comp) {
        components_.emplace_back(comp);
    }
}
