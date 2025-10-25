#include "GamePlayer.h"
#include "route/ProtocolRouter.h"
#include "Package.h"


using uranus::network::Package;

GamePlayer::GamePlayer() {

    router_ = std::make_unique<protocol::ProtocolRouter>(this);
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
    const auto *pkg = static_cast<Package *>(msg.data);
    router_->Dispatch(pkg->GetPackageID(), msg);
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