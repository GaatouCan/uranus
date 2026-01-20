#include "AppearanceComponent.h"
#include "GamePlayer.h"
#include "../../../ProtocolID.h"

#include <appearance.pb.h>

namespace gameplay {
    AppearanceComponent::AppearanceComponent(ComponentModule &module)
        : super(module),
          curAvatar_(0),
          curFrame_(0),
          curBackground_(0) {
    }

    AppearanceComponent::~AppearanceComponent() {
    }

    void AppearanceComponent::serialize_Appearance() {
        // TODO
    }

    void AppearanceComponent::deserialize_Appearance(const EntityList &list) {
        // TODO
    }

    void AppearanceComponent::onLogin() {
        sendInfo();
    }

    void AppearanceComponent::sendInfo() const {
        ::appearance::AppearanceInfo info;

        info.set_current_avatar(curAvatar_);
        info.set_current_frame(curFrame_);
        info.set_current_background(curBackground_);

        getPlayer().sendToClient(static_cast<int64_t>(protocol::ProtocolID::kAppearanceInfo), info);
    }
}
