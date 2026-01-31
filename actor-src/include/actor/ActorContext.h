#pragma once

#include "Package.h"
#include "DataAsset.h"

#include <base/noncopy.h>
#include <asio/any_completion_handler.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/any_io_executor.hpp>
#include <functional>
#include <chrono>
#include <map>


namespace uranus {
    class AttributeMap;
}

namespace uranus::actor {

    class ServerModule;
    class BaseActor;
    struct RepeatedTimerHandle;

    using std::unique_ptr;

    using DataAssetHandle   = unique_ptr<DataAsset>;
    using RepeatedTask      = std::function<void(BaseActor *)>;
    using ActorMap          = std::map<std::string, int64_t>;

    using SteadyTimePoint   = std::chrono::steady_clock::time_point;
    using SteadyDuration    = std::chrono::steady_clock::duration;
    using SessionHandler    = asio::any_completion_handler<void(PackageHandle)>;


    class ActorContext {

    public:
        ActorContext() = default;
        virtual ~ActorContext() = default;

        DISABLE_COPY_MOVE(ActorContext)

        virtual AttributeMap &attr() = 0;
        [[nodiscard]] virtual const AttributeMap &attr() const = 0;

        virtual asio::any_io_executor &executor() = 0;

        [[nodiscard]] virtual ServerModule *getModule(const std::string &name) const = 0;

        template<class T>
        requires std::derived_from<T, ServerModule>
        [[nodiscard]] T *getModuleT(const std::string &name) const;

        [[nodiscard]] virtual ActorMap getActorMap(const std::string &type) const = 0;
        [[nodiscard]] virtual int64_t queryActorId(const std::string &type, const std::string &name) const = 0;

        virtual void send(int ty, int64_t target, PackageHandle &&pkg) = 0;

        template<asio::completion_token_for<void(PackageHandle)> CompletionToken>
        auto call(int ty, int64_t target, PackageHandle &&req, CompletionToken &&token = asio::use_awaitable);

        virtual void listen(int64_t evt, bool cancel) = 0;
        virtual void dispatch(int64_t evt, DataAssetHandle &&data) = 0;

        virtual RepeatedTimerHandle createTimer(const RepeatedTask &task, SteadyDuration delay, SteadyDuration rate) = 0;
        virtual void cancelTimer(const RepeatedTimerHandle &handle) = 0;

    protected:
        virtual void createSession(int ty, int64_t target, PackageHandle &&req, SessionHandler &&handle) = 0;
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
