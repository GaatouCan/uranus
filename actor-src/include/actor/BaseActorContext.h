#pragma once

#include "ActorContext.h"
#include "Envelope.h"

#include <base/types.h>
#include <base/IdentAllocator.h>

#include <asio/co_spawn.hpp>
#include <atomic>
#include <memory>
#include <functional>


namespace uranus::actor {

    class BaseActor;

    using std::unique_ptr;
    using std::shared_ptr;
    using std::make_shared;
    using std::function;
    using asio::awaitable;
    using asio::co_spawn;

    using ActorDeleter  = function<void(BaseActor *)>;
    using ActorHandle   = unique_ptr<BaseActor, ActorDeleter>;


    class ACTOR_API BaseActorContext : public ActorContext, public std::enable_shared_from_this<BaseActorContext> {

        class ACTOR_API SessionNode final : public std::enable_shared_from_this<SessionNode> {

        public:
            SessionNode() = delete;

            SessionNode(asio::io_context &ctx, SessionHandle &&h, int64_t s);
            ~SessionNode();

            DISABLE_COPY_MOVE(SessionNode)

        public:
            asio::any_completion_handler<void(PackageHandle)> handle_;
            asio::executor_work_guard<asio::any_completion_executor> work_;
            SteadyTimer timer_;
            int64_t sess_;
            std::atomic_flag completed_;
        };

    public:
        BaseActorContext() = delete;

        BaseActorContext(asio::io_context &ctx, ActorHandle &&handle);
        ~BaseActorContext() override;

        DISABLE_COPY_MOVE(BaseActorContext)

        AttributeMap &attr() override;
        [[nodiscard]] const AttributeMap &attr() const override;

        virtual void run(DataAssetHandle &&data);
        virtual void terminate();

        [[nodiscard]] virtual bool isRunning() const;

        [[nodiscard]] BaseActor *getActor() const;

        template<class T>
        requires std::derived_from<T, BaseActor>
        T &getActor() const;

        void pushEvent(int64_t evt, unique_ptr<DataAsset> &&data);

        void pushEnvelope(Envelope &&envelope);

    protected:
        void createSession(int ty, int64_t target, PackageHandle &&req, SessionHandle &&handle) override;

        virtual void sendRequest(int ty, int64_t sess, int64_t target, PackageHandle &&pkg) = 0;
        virtual void sendResponse(int ty, int64_t sess, int64_t target, PackageHandle &&pkg) = 0;

    private:
        awaitable<void> process();

    private:
        asio::io_context &ctx_;
        ExecutorStrand strand_;

        ActorHandle handle_;

        std::atomic_flag running_;
        ConcurrentChannel<Envelope> mailbox_;

        IdentAllocator<int64_t, true> sessAlloc_;

        std::unordered_map<int64_t, shared_ptr<SessionNode>> sessions_;
        std::mutex sessMutex_;

        AttributeMap attr_;
    };

    template<class T>
    requires std::derived_from<T, BaseActor>
    T &BaseActorContext::getActor() const {
#ifndef NDEBUG
        assert(handle_ != nullptr);
#endif
        return dynamic_cast<T &>(*handle_.get());
    }
}
