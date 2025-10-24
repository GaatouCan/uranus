#include "GamePlayer.h"


extern "C" {
SERVICE_API AbstractPlayer *CreatePlayer() {
    return new GamePlayer();
}

SERVICE_API void DestroyPlayer(AbstractPlayer *player) {
    delete player;
}
}
