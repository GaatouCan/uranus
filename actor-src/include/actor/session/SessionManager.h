#pragma once

#include "../Package.h"

#include <base/noncopy.h>
#include <base/IdentAllocator.h>
#include <memory>
#include <unordered_map>
#include <asio/any_completion_handler.hpp>

namespace uranus::actor {

    using std::shared_ptr;
    using std::make_shared;
    using std::unordered_map;
    using SessionHandler = asio::any_completion_handler<void(PackageHandle)>;

    class BaseActorContext;
    class RequestSession;

    class ACTOR_API SessionManager final {

        friend class RequestSession;

    public:
        SessionManager() = delete;

        explicit SessionManager(BaseActorContext &ctx);
        ~SessionManager();

        DISABLE_COPY_MOVE(SessionManager)

        int64_t pushSession(SessionHandler &&handler);

        void dispatch(int64_t id, PackageHandle &&res);
        void cancel(int64_t id);

        void cancelAll();

    private:
        void removeOnComplete(int64_t id);

    private:
        BaseActorContext &ctx_;

        IdentAllocator<int64_t, false> alloc_;
        unordered_map<int64_t, shared_ptr<RequestSession>> sessions_;
    };
}
