#pragma once

#include "../../GamePlayer.h"

#include <actor/Package.h>

namespace gameplay::protocol {

    using uranus::actor::PackageHandle;

    void OnGreetingRequest(GamePlayer *plr, PackageHandle &&pkg);
}
