#pragma once

#include "Package.h"

#include <base/noncopy.h>
#include <base/types.h>
#include <base/AttributeMap.h>
#include <asio/co_spawn.hpp>
#include <asio/any_completion_handler.hpp>
#include <map>
#include <memory>
#include <functional>

#include <base/IdentAllocator.h>


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

        using SessionHandle = asio::any_completion_handler<void(PackageHandle)>;

        struct ACTOR_API SessionNode {
            asio::any_completion_handler<void(PackageHandle)> handle;
            asio::executor_work_guard<asio::any_completion_executor> work;
            uint32_t sess;

            SessionNode() = delete;

            SessionNode(SessionHandle &&h, uint32_t s);
        };

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

        virtual void run();
        virtual void terminate();

        [[nodiscard]] bool isRunning() const;

        /// Post message to this actor from other actor
        void pushEnvelope(Envelope &&envelope);

#pragma region For actor to call
        virtual ServerModule *getModule(const std::string &name) const = 0;
        virtual std::map<std::string, uint32_t> getServiceList() const = 0;

        /// Inner actor use this method to send message to other actor
        virtual void send(int ty, uint32_t target, PackageHandle &&pkg) = 0;

        /// Inner actor use this method to do async remote call to other actor
        auto remoteCall(uint32_t target, PackageHandle &&pkg) -> awaitable<PackageHandle>;
#pragma endregion

    protected:
        /// Implement this method to tell ActorContext how to post session package
        virtual bool call(uint32_t sess, uint32_t target, PackageHandle &&pkg) = 0;

        virtual void onErrorCode(std::error_code ec);
        virtual void onException(std::exception &e);

    private:
        awaitable<void> process();

    private:
        asio::io_context &ctx_;
        ActorHandle handle_;

        ConcurrentChannel<Envelope> mailbox_;

        IdentAllocator<uint32_t, true> sessAlloc_;

        std::mutex sessMutex_;
        std::unordered_map<uint32_t, SessionNode *> sessions_;

        AttributeMap attr_;
        uint32_t id_;
    };
}
