#pragma once

#include "AbstractPlayer.h"


using uranus::AbstractPlayer;
using uranus::DataAsset;
using uranus::Message;

namespace protocol {
    class ProtocolRouter;
}

class GamePlayer final : public AbstractPlayer {

public:
    GamePlayer();
    ~GamePlayer() override;

    int Initial(DataAsset *data) override;
    int Start() override;
    void Stop() override;

    void OnReceive(const Message &msg) override;
    void OnRequest(const Message &req, Message &res) override;

private:
    std::unique_ptr<protocol::ProtocolRouter> router_;
};
