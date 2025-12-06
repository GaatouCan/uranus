#pragma once

#include <base/noncopy.h>
#include <base/SharedLibrary.h>
#include <actor/BasePlayer.h>


using uranus::SharedLibrary;
using uranus::actor::BasePlayer;
using uranus::actor::ActorHandle;

class PlayerFactory final {

    using PlayerCreator = BasePlayer *(*)();
    using PlayerDestroyer = void (*)(BasePlayer *);

public:
    PlayerFactory();
    ~PlayerFactory();

    DISABLE_COPY_MOVE(PlayerFactory)

    void initial();

    ActorHandle create();
    void destroy(BasePlayer *plr);

private:
    SharedLibrary lib_;
    PlayerCreator creator_;
    PlayerDestroyer destroyer_;

};
