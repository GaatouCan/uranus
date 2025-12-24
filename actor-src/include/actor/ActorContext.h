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

        struct ACTOR_API SessionNode {
            asio::any_completion_handler<void(PackageHandle)> handle;
            asio::executor_work_guard<asio::any_completion_executor> work;
            uint32_t sess;

            SessionNode() = delete;

            SessionNode(asio::any_completion_handler<void(PackageHandle)> h, uint32_t s);
        };

        using SessionHandle = asio::any_completion_handler<void(PackageHandle)>;

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
        virtual void call(uint32_t sess, uint32_t target, PackageHandle &&pkg) = 0;

        virtual void onErrorCode(std::error_code ec) {}
        virtual void onException(std::exception &e) {}

        virtual std::map<std::string, uint32_t> getServiceList() const = 0;

        // template<asio::completion_token_for<void(PackageHandle)> Token>
        // auto asyncCall(uint32_t target, PackageHandle &&pkg, Token &&token);
        awaitable<PackageHandle> asyncCall(uint32_t target, PackageHandle &&pkg);

    private:
        awaitable<void> process();
        int64_t pushSession(SessionHandle &&handle);

    private:
        asio::io_context &ctx_;
        ActorHandle handle_;

        ConcurrentChannel<Envelope> mailbox_;

        IdentAllocator<uint32_t, true> sessIdAlloc_;

        std::mutex sessMutex_;
        std::unordered_map<uint32_t, SessionNode *> sessions_;

        AttributeMap attr_;
        uint32_t id_;
    };

    // template<asio::completion_token_for<void(PackageHandle)> Token>
    // auto ActorContext::asyncCall(uint32_t target, PackageHandle &&pkg, Token &&token) {
    //     return asio::async_initiate<Token, void(PackageHandle)>([this](
    //         asio::completion_handler_for<void(PackageHandle)> auto handler,
    //         uint32_t target, PackageHandle &&pkg
    //     ) {
    //         auto ret = this->pushSession(std::move(handler));
    //
    //         if (ret < 0)
    //             return;
    //
    //         const uint32_t sess = ret;
    //         this->call(sess, target, std::move(pkg));
    //
    //     }, token, target, std::move(pkg));
    // }
}
