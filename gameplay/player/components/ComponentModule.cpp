#include "ComponentModule.h"

namespace gameplay {

#define INTERNAL_COMPONENT_TABLE(comp, table, func) \
    {                                               \
        table,                                      \
        [comp]() {                                  \
            comp->serialize_##func();               \
        },                                          \
        [comp](const EntityList &val) {             \
            comp->deserialize_##func(val);          \
        }                                           \
    },

#define COMPONENT_TABLE(table, func) \
    INTERNAL_COMPONENT_TABLE(__temp, table, func)

#define REGISTER_COMPONENT(comp, ...)               \
    {                                               \
        auto __temp = (comp);                       \
        registerComponent(__temp, {__VA_ARGS__});   \
    }

    ComponentModule::ComponentModule(GamePlayer &plr)
        : owner_(plr),
#pragma region
          appearance_(*this)
#pragma endregion
    {
        REGISTER_COMPONENT(
            &appearance_,
            COMPONENT_TABLE("appearance", Appearance)
        )
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
