#pragma once

#include "ActorContext.h"
#include "../factory/ServiceHandle.h"

using uranus::ActorContext;
using uranus::AbstractActor;
using uranus::Message;

class GameWorld;

class ServiceContext final : public ActorContext {


public:
    explicit ServiceContext(GameWorld *world);
    ~ServiceContext() override;

    [[nodiscard]] AbstractActor *GetActor() const override;

    [[nodiscard]] Message *BuildMessage() override;

    void SendToService(int64_t target, Message *msg) override;
    void SendToService(const std::string &name, Message *msg) override;

    void SendToPlayer(int64_t pid, Message *msg) override;
    void SendToClient(int64_t pid, Message *msg) override;

    void PushMessage(Message *msg) override;

private:
    void SetUpService(ServiceHandle &&handle);

private:
    ServiceHandle handle_;
};