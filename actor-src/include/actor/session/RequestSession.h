#pragma once

#include "Package.h"

#include <base/types.h>
#include <base/noncopy.h>
#include <asio/any_completion_handler.hpp>
#include <asio/executor_work_guard.hpp>
#include <atomic>
#include <memory>

namespace uranus::actor {

    using SessionHandler = asio::any_completion_handler<void(PackageHandle)>;
    using SessionWorkGuard = asio::executor_work_guard<asio::any_completion_executor>;
    using std::atomic_flag;
    using std::shared_ptr;
    using std::weak_ptr;
    using std::enable_shared_from_this;

    class BaseActorContext;
    class SessionManager;

    class ACTOR_API RequestSession final : public enable_shared_from_this<RequestSession> {

        friend class SessionManager;

    public:
        RequestSession() = delete;

        RequestSession(const shared_ptr<BaseActorContext> &owner, SessionHandler &&handler, int64_t id);
        ~RequestSession();

        DISABLE_COPY_MOVE(RequestSession)

    private:
        void dispatch(PackageHandle &&res);
        void cancel();

    private:
        asio::any_io_executor exec_;

        int64_t id_;
        weak_ptr<BaseActorContext> owner_;

        SessionHandler handler_;
        SessionWorkGuard guard_;
        SteadyTimer timer_;
        atomic_flag completed_;
    };
}
