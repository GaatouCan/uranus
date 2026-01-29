#pragma once

#include "ActorContext.h"
#include "Envelope.h"

#include <base/types.h>
#include <base/IdentAllocator.h>

#include <asio/co_spawn.hpp>
#include <functional>


namespace uranus::actor {

    class BaseActor;

    using std::shared_ptr;
    using std::make_shared;
    using std::function;
    using std::atomic_flag;
    using asio::awaitable;
    using asio::co_spawn;

    using ActorDeleter  = function<void(BaseActor *)>;
    using ActorHandle   = unique_ptr<BaseActor, ActorDeleter>;


    class ACTOR_API BaseActorContext : public ActorContext, public std::enable_shared_from_this<BaseActorContext> {

        using SessionWorkGuard = asio::executor_work_guard<asio::any_completion_executor>;

        class ACTOR_API SessionNode final : public std::enable_shared_from_this<SessionNode> {

        public:
            SessionNode() = delete;

            SessionNode(asio::io_context &ctx, SessionHandler &&h, int64_t s);
            ~SessionNode();

            DISABLE_COPY_MOVE(SessionNode)

            void dispatch(PackageHandle &&res);
            void cancel();

        public:
            SessionHandler      handler_;
            SessionWorkGuard    guard_;
            SteadyTimer         timer_;
            int64_t             sess_;
            atomic_flag         completed_;
        };

        using SessionMap = std::unordered_map<int64_t, shared_ptr<SessionNode>>;

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

        void pushEnvelope(Envelope &&envelope);

    protected:
        void createSession(int ty, int64_t target, PackageHandle &&req, SessionHandler &&handle) override;

        virtual void sendRequest(int ty, int64_t sess, int64_t target, PackageHandle &&pkg) = 0;
        virtual void sendResponse(int ty, int64_t sess, int64_t target, PackageHandle &&pkg) = 0;

        virtual void onErrorCode(std::error_code ec);
        virtual void onException(std::exception &e);

        virtual void cleanUp();

    private:
        awaitable<void> process();
        awaitable<void> tick();

    private:
        asio::io_context &ctx_;
        ExecutorStrand strand_;

        AttributeMap attr_;
        ActorHandle handle_;

        atomic_flag running_;

        ConcurrentChannel<Envelope> mailbox_;
        SteadyTimer ticker_;

        IdentAllocator<int64_t, true> sessAlloc_;
        SessionMap sessions_;
        std::mutex sessMutex_;
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
