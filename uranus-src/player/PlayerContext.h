#pragma once

#include "ActorContext.h"
#include "../factory/PlayerHandle.h"


using uranus::DataAsset;
using uranus::ActorContext;
using uranus::GameServer;
using uranus::Message;
using uranus::AbstractActor;
using uranus::AbstractPlayer;


class PlayerContext final : public ActorContext {

    friend class PlayerManager;

public:
    explicit PlayerContext(GameServer *ser);
    ~PlayerContext() override;

    [[nodiscard]] AbstractActor *GetActor() const override;

    int Initial(DataAsset *data) override;
    int Start() override;

    void SendToService(int64_t target, Message *msg) override;
    void SendToService(const std::string &name, Message *msg) override;

    void SendToPlayer(int64_t pid, Message *msg) override;
    void SendToClient(int64_t pid, Message *msg) override;

    void PushMessage(Message *msg) override;

protected:
    void CleanUp() override;

private:
    void SetUpPlayer(PlayerHandle &&handle);

private:
    PlayerHandle handle_;
};
