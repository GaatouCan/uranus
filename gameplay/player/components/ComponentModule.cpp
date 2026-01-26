#include "ComponentModule.h"
#include <base/utils.h>

namespace gameplay {

#define SERIALIZE_COMPONENT(comp, table, func)  \
do {                                            \
    nlohmann::json val;                         \
    val["table"] = #table;                      \
    (comp).serialize_##func(val["data"]);       \
    temp.push_back(val);                        \
} while (false);

#define DESERIALIZE_COMPONENT(comp, table, func)    \
if (val["table"].get<std::string>() == #table) {    \
    (comp).deserialize_##func(val["data"]);         \
}

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

    void ComponentModule::serialize(nlohmann::json &data) const {
        auto &temp = data["component"];

        SERIALIZE_COMPONENT(appearance_, appearance, Appearance)
    }

    void ComponentModule::deserialize(const nlohmann::json &data) {
        for (const auto &val : data) {
            DESERIALIZE_COMPONENT(appearance_, appearance, Appearance)
            // Other components
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

#undef SERIALIZE_COMPONENT
#undef DESERIALIZE_COMPONENT
}
