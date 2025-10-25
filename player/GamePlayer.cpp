#include "GamePlayer.h"


GamePlayer::GamePlayer() {
}

GamePlayer::~GamePlayer() {
}

int GamePlayer::Initial(DataAsset *data) {
    return 1;
}

int GamePlayer::Start() {
    return 1;
}

void GamePlayer::Stop() {

}

void GamePlayer::OnReceive(const Message &msg) {

}

void GamePlayer::OnRequest(const Message &req, Message &res) {

}

extern "C" {
    SERVICE_API AbstractPlayer *CreatePlayer() {
        return new GamePlayer();
    }

    SERVICE_API void DestroyPlayer(AbstractPlayer *player) {
        delete player;
    }
}