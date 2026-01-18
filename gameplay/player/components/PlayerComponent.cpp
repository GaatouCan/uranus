#include "PlayerComponent.h"

namespace gameplay {
    PlayerComponent::PlayerComponent(ComponentModule &module)
        : module_(module) {
    }

    PlayerComponent::~PlayerComponent() {
    }

    ComponentModule &PlayerComponent::getComponentModule() const {
        return module_;
    }

    void PlayerComponent::onLogin() {
    }

    void PlayerComponent::onLogout() {
    }
}
