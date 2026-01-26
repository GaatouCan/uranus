#include "ComponentModule.h"
#include <base/utils.h>

namespace gameplay {

#define SERIALIZE_COMPONENT(comp, table, func)  \
do {                                            \
    auto &data = j[table];                      \
    (comp).serialize_##func(data);              \
} while (false);

#define DESERIALIZE_COMPONENT(comp, table, func) \
    case table##_t : (comp).deserialize_##func(val["data"]); break;

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
        nlohmann::json j;

        SERIALIZE_COMPONENT(appearance_, "appearance", Appearance)

        // auto bytes = nlohmann::json::to_bson(j);
        // TODO: Deal with the json object
    }

    void ComponentModule::deserialize(const nlohmann::json &data) {
        for (const auto &val : data) {
            if (val["table"].get<std::string>() == "appearance") {
                appearance_.deserialize_Appearance(val["data"]);
            }
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
