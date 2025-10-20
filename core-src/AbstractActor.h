#pragma once


#include "ServerModule.h"
#include "GameServer.h"


namespace uranus {

    class DataAsset;
    struct Message;

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


        template<class T>
        requires std::is_base_of_v<ServerModule, T>
        T *GetServerModule() const;

    private:
        void SetUpContext(ActorContext *ctx);

    private:
        ActorContext *ctx_;
    };

    template<class T> requires std::is_base_of_v<ServerModule, T>
    T *AbstractActor::GetServerModule() const {
        return this->GetGameServer()->GetModule<T>();
    }
}
