#pragma once

#include "GamePlayer.h"

#include <actor/Package.h>

namespace gameplay::protocol {

    using uranus::actor::PackageHandle;

    void Route_GreetingRequest(GamePlayer *plr, PackageHandle &&pkg);

    PackageHandle Request_PlayerInfoRequest(GamePlayer *plr, PackageHandle &&pkg);
}
