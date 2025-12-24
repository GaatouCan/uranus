#pragma once

#include "Package.h"

#include <base/noncopy.h>
#include <base/types.h>
#include <base/AttributeMap.h>
#include <asio/co_spawn.hpp>
#include <map>
#include <memory>
#include <functional>


namespace uranus::actor {

    class BaseActor;
    class ServerModule;

    using std::unique_ptr;
    using std::function;
    using asio::awaitable;
    using asio::co_spawn;

    using ActorDeleter = function<void(BaseActor *)>;
    using ActorHandle = unique_ptr<BaseActor, ActorDeleter>;

    class ACTOR_API ActorContext : public std::enable_shared_from_this<ActorContext> {

    public:
        ActorContext() = delete;

        explicit ActorContext(asio::io_context &ctx);
        virtual ~ActorContext();

        DISABLE_COPY_MOVE(ActorContext)

        void setId(uint32_t id);
        [[nodiscard]] uint32_t getId() const;
        [[nodiscard]] AttributeMap &attr();

        void setUpActor(ActorHandle &&handle);
        [[nodiscard]] BaseActor *getActor() const;

        virtual ServerModule *getModule(const std::string &name) const = 0;

        virtual void run();
        virtual void terminate();

        [[nodiscard]] bool isRunning() const;

        void pushEnvelope(Envelope &&envelope);

        virtual void send(int ty, uint32_t target, PackageHandle &&pkg) = 0;

        virtual void onErrorCode(std::error_code ec) = 0;
        virtual void onException(std::exception &e) = 0;

        virtual std::map<std::string, uint32_t> getServiceList() const = 0;

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
