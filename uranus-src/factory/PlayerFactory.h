#pragma once


#include "../other/SharedLibrary.h"
#include "PlayerHandle.h"


class PlayerFactory final {

    using PlayerCreator = AbstractPlayer *(*)();
    using PlayerDestroyer = void (*)(AbstractPlayer *);

public:
    PlayerFactory();
    ~PlayerFactory();

    DISABLE_COPY_MOVE(PlayerFactory)

    [[nodiscard]] PlayerHandle CreatePlayer();

    void DestroyPlayer(AbstractPlayer *pPlayer);

private:
    SharedLibrary library_;

    PlayerCreator creator_;
    PlayerDestroyer destroyer_;
};
