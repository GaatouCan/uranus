#pragma once

#include "ActorContext.h"
#include "Envelope.h"
#include "timer/TimerManager.h"
#include "session/SessionManager.h"


namespace uranus::actor {

    using std::shared_ptr;
    using std::make_shared;
    using std::function;
    using std::atomic_flag;
    using asio::awaitable;

    using ActorDeleter  = function<void(BaseActor *)>;
    using ActorHandle   = unique_ptr<BaseActor, ActorDeleter>;

    class BaseActor;
    class RepeatedTimer;
    class RequestSession;


    class ACTOR_API BaseActorContext : public ActorContext, public std::enable_shared_from_this<BaseActorContext> {

        friend class RepeatedTimer;
        friend class RequestSession;

    public:
        BaseActorContext() = delete;

        BaseActorContext(asio::any_io_executor exec, ActorHandle &&actor);
        ~BaseActorContext() override;

        DISABLE_COPY_MOVE(BaseActorContext)

        AttributeMap &attr() override;
        [[nodiscard]] const AttributeMap &attr() const override;

        virtual void run(DataAssetHandle &&data);
        virtual void terminate();

        [[nodiscard]] virtual bool isInitial() const;
        [[nodiscard]] virtual bool isRunning() const;
        [[nodiscard]] virtual bool isTerminated() const;

        asio::any_io_executor &executor() override;

        [[nodiscard]] BaseActor *getActor() const;

        template<class T>
        requires std::derived_from<T, BaseActor>
        T &getActor() const;

        void pushEnvelope(Envelope &&envelope);

        RepeatedTimerHandle createTimer(const RepeatedTask &task, SteadyDuration delay, SteadyDuration rate) override;
        void cancelTimer(const RepeatedTimerHandle &handle) override;

    protected:
        void createSession(int ty, int64_t target, PackageHandle &&req, SessionHandler &&handle) override;

        virtual void sendRequest(int ty, int64_t sess, int64_t target, PackageHandle &&pkg) = 0;
        virtual void sendResponse(int ty, int64_t sess, int64_t target, PackageHandle &&pkg) = 0;

        virtual void onErrorCode(std::error_code ec);
        virtual void onException(std::exception &e);

        virtual bool cleanUp();

    private:
        awaitable<void> process();
        awaitable<void> tick();

    private:
        asio::any_io_executor exec_;

        AttributeMap attr_;
        ActorHandle handle_;

        atomic_flag running_;
        atomic_flag terminated_;

        ConcurrentChannel<Envelope> mailbox_;
        SteadyTimer ticker_;

        SessionManager sessionManager_;
        TimerManager timerManager_;
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
