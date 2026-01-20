#include "ComponentModule.h"
#include <base/utils.h>

namespace gameplay {

#define DESERIALIZE_COMPONENT(comp, table, func) \
    case table##_t : (comp).deserialize_##func(list); break;

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

    void ComponentModule::serialize() {
    }

    void ComponentModule::deserialize(const EntitiesMap &entities) {
        using uranus::utils::StringToTag;
        using uranus::utils::udl::operator ""_t;

        for (const auto &[table, list] : entities) {
            switch (StringToTag(table)) {
                DESERIALIZE_COMPONENT(appearance_, "appearance", Appearance)
                default: break;
            }
        }
    }

    void ComponentModule::onLogin() const {
        for (const auto comp : components_) {
            comp->onLogin();
        }
    }

    void ComponentModule::onLogout() const {
        for (const auto comp : components_) {
            comp->onLogout();
        }
    }

    void ComponentModule::registerComponent(PlayerComponent *comp) {
        components_.emplace_back(comp);
    }
}
