#pragma once

#include "Package.h"

#include <base/noncopy.h>
#include <base/AttributeMap.h>
#include <asio/any_completion_handler.hpp>
#include <asio/use_awaitable.hpp>

namespace uranus::actor {

    class ServerModule;

    class ActorContext {

        using SessionHandle = asio::any_completion_handler<void(PackageHandle)>;

    public:
        ActorContext() = default;
        virtual ~ActorContext() = default;

        DISABLE_COPY_MOVE(ActorContext)

        virtual AttributeMap &attr() = 0;

        [[nodiscard]] virtual ServerModule *getModule(const std::string &name) const = 0;

        virtual void send(int ty, int64_t target, PackageHandle &&pkg) = 0;

        template<asio::completion_token_for<void(PackageHandle)> CompletionToken>
        auto call(int ty, int64_t target, PackageHandle &&req, CompletionToken &&token = asio::use_awaitable);

    protected:
        virtual void createSession(int ty, int64_t target, PackageHandle &&req, SessionHandle &&handle) = 0;

    };

    template<asio::completion_token_for<void(PackageHandle)> CompletionToken>
    auto ActorContext::call(int ty, int64_t target, PackageHandle &&req, CompletionToken &&token) {
        return asio::async_initiate<CompletionToken, void(PackageHandle)>([this](
            asio::completion_handler_for<void(PackageHandle)> auto handler,
            const int type,
            const int64_t dest,
            PackageHandle &&temp
        ) mutable {
            this->createSession(type, dest, std::move(temp), std::move(handler));
        }, token, ty, target, std::move(req));
    }
}
