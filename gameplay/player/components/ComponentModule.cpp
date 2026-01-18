#include "ComponentModule.h"

namespace gameplay {
    ComponentModule::ComponentModule(GamePlayer &plr)
        : owner_(plr) {
    }

    ComponentModule::~ComponentModule() {
    }

    GamePlayer &ComponentModule::getPlayer() const {
        return owner_;
    }
}
