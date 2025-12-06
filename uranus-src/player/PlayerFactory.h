#pragma once

#include <base/noncopy.h>
#include <base/SharedLibrary.h>


class PlayerFactory final {

    PlayerFactory();
public:
    ~PlayerFactory();

    DISABLE_COPY_MOVE(PlayerFactory)

    static PlayerFactory &instance();
};
