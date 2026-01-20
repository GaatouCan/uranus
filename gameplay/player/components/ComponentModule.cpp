#include "ComponentModule.h"

namespace gameplay {

#define REGISTER_COMPONENT(comp)

    ComponentModule::ComponentModule(GamePlayer &plr)
        : owner_(plr),
#pragma region
          appearance_(*this)
#pragma endregion
    {
        registerComponent(&appearance_, {
            {"appearance",
                [comp = &appearance_]() {
                    comp->serialize_Appearance();
                },
                [comp = &appearance_](const EntityList &val) {
                    comp->deserialize_Appearance(val);
            }}
        });
    }

    ComponentModule::~ComponentModule() {
    }

    GamePlayer &ComponentModule::getPlayer() const {
        return owner_;
    }

    void ComponentModule::serialize() {
    }

    void ComponentModule::deserialize(const EntitiesMap &entities) {
        for (const auto &[table, val] : entities) {
            if (const auto it = deserFuncs_.find(table); it != deserFuncs_.end()) {
                std::invoke(it->second, val);
            }
        }
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

    void ComponentModule::registerComponent(PlayerComponent *comp, const vector<RegisterData> &list) {
        components_.emplace_back(comp);
        for (const auto &val : list) {
            deserFuncs_.insert_or_assign(val.table, val.deser);
        }
    }
}
