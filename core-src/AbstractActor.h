#pragma once

#include "Common.h"


class Package;
class GameServer;

class CORE_API AbstractActor {

    friend class ActorContext;

public:
    AbstractActor();
    virtual ~AbstractActor();

    DISABLE_COPY_MOVE(AbstractActor)

    virtual void OnPackage(Package *pkg);

protected:
    [[nodiscard]] ActorContext *GetActorContext() const;
    [[nodiscard]] GameServer *GetGameServer() const;

private:
    void SetUpContext(ActorContext *ctx);

private:
    ActorContext *ctx_;
};
