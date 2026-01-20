#pragma once

#include "GamePlayer.h"

#include <actor/Package.h>

namespace gameplay::protocol {
    void Route_LoginDataResult(GamePlayer *plr, PackageHandle &&pkg);
}