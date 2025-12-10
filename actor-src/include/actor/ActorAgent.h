#pragma once

#include "BaseActor.h"
#include "Package.h"

#include <base/types.h>
#include <base/AttributeMap.h>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>


namespace uranus::actor {

    using asio::awaitable;
    using asio::co_spawn;
    using asio::detached;

    using std::tuple;
    using std::error_code;
    using std::shared_ptr;
    using std::make_shared;
    using std::enable_shared_from_this;

    class ACTOR_API ActorAgent {

    public:
        ActorAgent() = delete;

        explicit ActorAgent(asio::io_context &ctx);
        virtual ~ActorAgent();

        DISABLE_COPY_MOVE(ActorAgent)

        void setActor(ActorHandle &&handle);
        [[nodiscard]] BaseActor *getActor() const;

        void setId(uint32_t id);
        [[nodiscard]] uint32_t getId() const;

        [[nodiscard]] asio::io_context &getIOContext() const;

        [[nodiscard]] bool isRunning() const;

        AttributeMap &attr();

        virtual void run() = 0;
        virtual void terminate() = 0;

        void pushEnvelope(Envelope &&envelope);

        virtual void sendMessage(int32_t ty, uint32_t target, MessageHandle &&msg) = 0;
        virtual void sendMessage(int32_t ty, uint32_t target, Message *msg) = 0;

    protected:
        asio::io_context &ctx_;
        ConcurrentChannel<Envelope> mailbox_;

        ActorHandle actor_;
        AttributeMap attr_;

        uint32_t id_;
    };
}
