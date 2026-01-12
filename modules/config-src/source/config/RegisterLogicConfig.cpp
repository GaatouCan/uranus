#include "ConfigModule.h"

#pragma region LogicConfig include here

#include "logic/LC_Appearance.h"

#pragma endregion

namespace uranus::config {

#define REGISTER_LOGIC(type, path, logic)                                       \
{                                                                               \
    pathMap_.insert_or_assign(LogicConfigType::type, path);                     \
    logicMap_.insert_or_assign(LogicConfigType::type, make_unique<logic>());    \
}

    void ConfigModule::registerLogicConfig() {
        REGISTER_LOGIC(kAvatar, "config/json/appear", LC_Appearance)
    }

#undef REGISTER_LOGIC
}
