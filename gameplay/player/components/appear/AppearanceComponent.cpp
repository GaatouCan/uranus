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

    void AppearanceComponent::sendInfo() const {
        ::appearance::AppearanceInfo info;

        info.set_current_avatar(curAvatar_);
        info.set_current_frame(curFrame_);
        info.set_current_background(curBackground_);

        SEND_TO_CLIENT(getPlayer(), kAppearanceInfo, info)
    }
}
