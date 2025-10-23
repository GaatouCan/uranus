#pragma once

#include "Message.h"
#include "GameServer.h"


namespace uranus {

    class DataAsset;

    /// The base abstract actor
    class CORE_API AbstractActor {

        friend class ActorContext;

    public:
        AbstractActor();
        virtual ~AbstractActor();

        DISABLE_COPY_MOVE(AbstractActor)

        virtual int Initial(DataAsset *data);
        virtual int Start();
        virtual void Stop();

        /// Handle while receive message
        virtual void OnReceive(const Message &msg);

        /// Handle while receive request from other actor,
        /// and write back to the res
        virtual void OnRequest(const Message &req, Message &res);

    protected:
        /// Get the ActorContext which manages this actor
        [[nodiscard]] ActorContext *GetActorContext() const;

        /// Get the pointer to the GameServer
        [[nodiscard]] GameServer *GetGameServer() const;

        template<class T>
        requires std::is_base_of_v<ServerModule, T>
        T *GetServerModule() const;

    private:
        void SetUpContext(ActorContext *ctx);

    private:
        /// The pointer to the context manages this actor
        ActorContext *ctx_;
    };

    template<class T> requires std::is_base_of_v<ServerModule, T>
    T *AbstractActor::GetServerModule() const {
        return this->GetGameServer()->GetModule<T>();
    }
}
