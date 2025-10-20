#pragma once

#include "ActorContext.h"

using uranus::DataAsset;
using uranus::ActorContext;
using uranus::GameServer;
using uranus::Message;
using uranus::AbstractActor;

class PlayerContext final : public ActorContext {

public:
    explicit PlayerContext(GameServer *ser);
    ~PlayerContext() override;

    [[nodiscard]] AbstractActor *GetActor() const override;

    int Initial(DataAsset *data) override;
    int Start() override;
    void Stop() override;

    void SendToService(int64_t target, Message *msg) override;
    void SendToService(const std::string &name, Message *msg) override;

    void SendToPlayer(int64_t pid, Message *msg) override;
    void SendToClient(int64_t pid, Message *msg) override;

    void PushMessage(Message *msg) override;
};
