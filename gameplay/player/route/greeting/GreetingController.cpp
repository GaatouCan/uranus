#include "GreetingController.h"
#include "../../../ProtocolID.h"

#include <greeting.pb.h>
#include <spdlog/fmt/fmt.h>

namespace gameplay::protocol {

    using uranus::actor::Package;

    void OnGreetingRequest(GamePlayer *plr, PackageHandle &&pkg) {

        greeting::GreetingRequest req;
        req.ParseFromArray(pkg->payload_.data(), pkg->payload_.size());

        const auto data = req.data();

        greeting::GreetingResponse res;
        res.set_data(fmt::format("Echo: {}", data));

        auto rPkg = Package::getHandle();
        rPkg->setId(protocol::kGreetingResponse);
        rPkg->setData(res.SerializeAsString());

        plr->sendToClient(std::move(rPkg));
    }
}
