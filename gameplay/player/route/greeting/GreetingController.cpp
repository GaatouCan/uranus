#include "GreetingController.h"
#include "common/ProtocolID.h"

#include <greeting.pb.h>
#include <spdlog/fmt/fmt.h>

namespace gameplay::protocol {

    using uranus::actor::Package;

    void Route_GreetingRequest(GamePlayer *plr, PackageHandle &&pkg) {

        greeting::GreetingRequest req;
        req.ParseFromArray(pkg->payload_.data(), pkg->payload_.size());

        const auto data = req.data();

        greeting::GreetingResponse res;
        res.set_data(fmt::format("Echo: {}", data));

        plr->sendToClient(kGreetingResponse, res);
    }

    PackageHandle Request_PlayerInfoRequest(GamePlayer *plr, PackageHandle &&pkg) {
        greeting::PlayerInfoResponse res;

        {
            const auto &appear = plr->getComponentModule().getAppearance();

            res.set_current_avatar(appear.getCurrentAvatar());
            res.set_current_frame(appear.getCurrentFrame());
            res.set_current_background(appear.getCurrentBackground());
        }

        auto rPkg = Package::getHandle();
        rPkg->setId(kPlayerInfoResponse);
        rPkg->payload_.resize(res.ByteSizeLong());
        res.SerializeToArray(rPkg->payload_.data(), rPkg->payload_.size());

        return rPkg;
    }
}
