#include "AppearanceController.h"

#include <appearance.pb.h>

namespace gameplay::protocol {
    void Route_AppearanceRequest(GamePlayer *plr, PackageHandle &&pkg) {
        auto &comp = plr->getComponentModule().getAppearance();

        appearance::AppearanceRequest req;
        req.ParseFromArray(pkg->payload_.data(), pkg->payload_.size());

        switch (req.op()) {
            case appearance::AppearanceRequest::INFO_REQUEST: {
                comp.sendInfo();
            }
            break;
            case appearance::AppearanceRequest::CHANGE_AVATAR: {
                // TODO
            }
            break;
            default: ;
        }
    }
}
