#include "ConfigModule.h"

#pragma region LogicConfig include here

#include "logic/LC_Avatar.h"

#pragma endregion

namespace uranus::config {

#define REGISTER_LOGIC(type, path, logic)                                       \
{                                                                               \
    pathMap_.insert_or_assign(LogicConfigType::type, path);                     \
    logicMap_.insert_or_assign(LogicConfigType::type, make_unique<logic>());    \
}

    void ConfigModule::registerLogicConfig() {
        REGISTER_LOGIC(kAvatar, "config/avatar", LC_Avatar)
    }

#undef REGISTER_LOGIC
}
