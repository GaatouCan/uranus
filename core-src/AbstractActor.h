#pragma once

#include "Common.h"


namespace uranus {

    class DataAsset;
    class Message;
    class GameServer;

    class CORE_API AbstractActor {

        friend class ActorContext;

    public:
        AbstractActor();
        virtual ~AbstractActor();

        DISABLE_COPY_MOVE(AbstractActor)

        virtual int Initial(DataAsset *data);
        virtual int Start();
        virtual void Stop();

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
