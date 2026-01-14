#include "GreetingController.h"
#include "../../../ProtocolID.h"

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

        auto rPkg = Package::getHandle();

        rPkg->setId(static_cast<int64_t>(ProtocolID::kGreetingRequest));
        rPkg->setData(res.SerializeAsString());

        plr->sendToClient(std::move(rPkg));
    }
}
