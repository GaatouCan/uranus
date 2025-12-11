#pragma once

#include "BaseActor.h"
#include "AgentPipeline.h"

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

    class ACTOR_API ActorAgent final : public enable_shared_from_this<ActorAgent> {

    public:
        ActorAgent() = delete;

        explicit ActorAgent(asio::io_context &ctx);
        ~ActorAgent();

        DISABLE_COPY_MOVE(ActorAgent)

        void setActor(ActorHandle &&handle);
        [[nodiscard]] BaseActor *getActor() const;

        void setId(uint32_t id);
        [[nodiscard]] uint32_t getId() const;

        [[nodiscard]] asio::io_context &getIOContext() const;

        [[nodiscard]] bool isRunning() const;

        AttributeMap &attr();

        void run();
        void terminate();

        void pushEnvelope(Envelope &&envelope);

        void send(int32_t ty, uint32_t target, PackageHandle &&pkg);
        void send(int32_t ty, uint32_t target, Package *pkg);

    private:
        awaitable<void> process();

    protected:
        asio::io_context &ctx_;
        ConcurrentChannel<Envelope> mailbox_;

        ActorHandle actor_;
        AttributeMap attr_;
        AgentPipeline pipeline_;

        uint32_t id_;
    };
}
