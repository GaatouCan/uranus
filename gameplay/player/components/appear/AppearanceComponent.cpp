#include "AppearanceComponent.h"
#include "GamePlayer.h"
#include "../../../common/ProtocolID.h"

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

    void AppearanceComponent::serialize_Appearance(nlohmann::json &data) const {
        data["current_avatar"]      = curAvatar_;
        data["current_frame"]       = curFrame_;
        data["current_background"]  = curBackground_;
    }

    void AppearanceComponent::deserialize_Appearance(const nlohmann::json &data) {
        curAvatar_      = data["current_avatar"];
        curFrame_       = data["current_frame"];
        curBackground_  = data["current_background"];
    }

    void AppearanceComponent::onLogin() {
        sendInfo();
    }

    void AppearanceComponent::sendInfo() const {
        ::appearance::AppearanceInfo info;

        info.set_current_avatar(curAvatar_);
        info.set_current_frame(curFrame_);
        info.set_current_background(curBackground_);

        getPlayer().sendToClient(protocol::ProtocolID::kAppearanceInfo, info);
    }
}
