#include "PlayerComponent.h"
#include "ComponentModule.h"
#include "GamePlayer.h"

namespace gameplay {
    PlayerComponent::PlayerComponent(ComponentModule &module)
        : module_(module) {
    }

    PlayerComponent::~PlayerComponent() {
    }

    ComponentModule &PlayerComponent::getComponentModule() const {
        return module_;
    }

    GamePlayer &PlayerComponent::getPlayer() const {
        return module_.getPlayer();
    }

    int64_t PlayerComponent::getPlayerId() const {
        return getPlayer().getPlayerId();
    }

    void PlayerComponent::onLogin() {
    }

    void PlayerComponent::onLogout() {
    }
}
