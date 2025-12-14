#pragma once

#include "Package.h"

#include <base/noncopy.h>
#include <base/types.h>
#include <base/AttributeMap.h>
#include <memory>
#include <functional>
#include <asio/co_spawn.hpp>

namespace uranus::actor {

    class Actor;

    using std::unique_ptr;
    using std::function;
    using asio::awaitable;
    using asio::co_spawn;

    using ActorDeleter = function<void(Actor *)>;
    using ActorHandle = unique_ptr<Actor, ActorDeleter>;

    class ACTOR_API ActorContext : public std::enable_shared_from_this<ActorContext> {

    public:
        ActorContext() = delete;

        explicit ActorContext(asio::io_context &ctx);
        virtual ~ActorContext();

        DISABLE_COPY_MOVE(ActorContext)

        void setId(uint32_t id);
        [[nodiscard]] uint32_t getId() const;

        void setUpActor(ActorHandle &&handle);
        [[nodiscard]] Actor *getActor() const;

        virtual void run();
        virtual void terminate();

        [[nodiscard]] AttributeMap &attr();

        [[nodiscard]] bool isRunning() const;

        void pushEnvelope(Envelope &&envelope);

        virtual void send(int ty, uint32_t target, PackageHandle &&pkg) = 0;

    private:
        awaitable<void> process();

    private:
        asio::io_context &ctx_;
        ActorHandle handle_;

        ConcurrentChannel<Envelope> mailbox_;

        AttributeMap attr_;
        uint32_t id_;
    };
}
