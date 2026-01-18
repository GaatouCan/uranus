#pragma once

#include "GamePlayer.h"

#include <actor/Package.h>

namespace gameplay::protocol {

    using uranus::actor::PackageHandle;

    void Route_AppearanceRequest(GamePlayer *plr, PackageHandle &&pkg);
}