#include "LoginController.h"

#include <nlohmann/json.hpp>

namespace gameplay::protocol {
    void Route_LoginDataResult(GamePlayer *plr, PackageHandle &&pkg) {
        const auto data = nlohmann::json::parse(pkg->payload_.begin(), pkg->payload_.end());
        plr->getComponentModule().deserialize(data);
        plr->onLogin();
    }
}