#pragma once

#include "Common.h"


namespace uranus {

    class Message;
    class GameServer;

    class CORE_API AbstractActor {

        friend class ActorContext;

    public:
        AbstractActor();
        virtual ~AbstractActor();

        DISABLE_COPY_MOVE(AbstractActor)

        virtual void Initial();

        virtual void OnReceive(Message *msg);

    protected:
        [[nodiscard]] ActorContext *GetActorContext() const;
        [[nodiscard]] GameServer *GetGameServer() const;

    private:
        void SetUpContext(ActorContext *ctx);

    private:
        ActorContext *ctx_;
    };
}