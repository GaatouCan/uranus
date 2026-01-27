#pragma once

#include "Package.h"
#include "DataAsset.h"
#include "ServerModule.h"

#include <base/AttributeMap.h>
#include <asio/any_completion_handler.hpp>
#include <asio/use_awaitable.hpp>
#include <map>

namespace uranus::actor {

    using std::unique_ptr;
    using ServiceMap = std::map<std::string, int64_t>;

    class ActorContext {

    protected:
        using SessionHandle = asio::any_completion_handler<void(PackageHandle)>;

    public:
        ActorContext() = default;
        virtual ~ActorContext() = default;

        DISABLE_COPY_MOVE(ActorContext)

        virtual AttributeMap &attr() = 0;
        [[nodiscard]] virtual const AttributeMap &attr() const = 0;

        [[nodiscard]] virtual ServerModule *getModule(const std::string &name) const = 0;

        template<class T>
        requires std::derived_from<T, ServerModule>
        [[nodiscard]] T *getModuleT(const std::string &name) const;

        [[nodiscard]] virtual ServiceMap getServiceMap() const = 0;
        [[nodiscard]] virtual int64_t queryServiceId(const std::string &name) const = 0;

        virtual void send(int ty, int64_t target, PackageHandle &&pkg) = 0;
        virtual void dispatch(int ty, int64_t target, int64_t evt, unique_ptr<DataAsset> &&data) = 0;

        template<asio::completion_token_for<void(PackageHandle)> CompletionToken>
        auto call(int ty, int64_t target, PackageHandle &&req, CompletionToken &&token = asio::use_awaitable);

    protected:
        virtual void createSession(int ty, int64_t target, PackageHandle &&req, SessionHandle &&handle) = 0;
    };

    template<class T>
    requires std::derived_from<T, ServerModule>
    T *ActorContext::getModuleT(const std::string &name) const {
        if (auto *module = this->getModule(name); module != nullptr) {
            return dynamic_cast<T *>(module);
        }
        return nullptr;
    }

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
