#pragma once

#include "Actor.h"

#include <memory>
#include <asio.hpp>
#include <asio/experimental/concurrent_channel.hpp>


namespace uranus::actor {

    using default_token = asio::as_tuple_t<asio::use_awaitable_t<>>;

    using asio::awaitable;
    using asio::co_spawn;
    using asio::detached;

    using std::tuple;
    using std::error_code;

    template<class T>
    using ConcurrentChannel = default_token::as_default_on_t<asio::experimental::concurrent_channel<void(error_code, T)>>;

    class ActorContext {

    public:
        ActorContext() = default;
        virtual ~ActorContext() = default;

        DISABLE_COPY_MOVE(ActorContext)

        virtual void sendMessage(MessageHandle &&msg) = 0;
        virtual void sendMessage(Message *msg) = 0;
    };

    template<class T>
    requires std::is_base_of_v<Message, T>
    class ActorContextRouter {
    public:
        using Type = T;
        using HandleType = Message::Pointer<Type>;

        ActorContextRouter() = delete;

        explicit ActorContextRouter(ActorContext &ctx);
        virtual ~ActorContextRouter();

        DISABLE_COPY_MOVE(ActorContextRouter)

        [[nodiscard]] ActorContext &getContext() const;

        virtual void sendMessage(HandleType &&msg) = 0;

    protected:
        ActorContext &ctx_;
    };

    template<class Router>
    requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
    class ActorContextImpl : public ActorContext, public std::enable_shared_from_this<ActorContextImpl<Router>> {

    public:
        using MessageType = Router::Type;
        using MessageHandleType = Router::HandleType;

        ActorContextImpl() = delete;

        explicit ActorContextImpl(asio::io_context &ctx);
        ~ActorContextImpl() override;

        DISABLE_COPY_MOVE(ActorContextImpl)

        [[nodiscard]] asio::io_context &getIOContext() const;

        Router &getRouter();

        void setActor(ActorHandle &&handle);
        [[nodiscard]] Actor *getActor() const;

        void sendMessage(MessageHandle &&msg) override;
        void sendMessage(Message *msg) override;

    private:
        asio::io_context &ctx_;
        ConcurrentChannel<MessageHandleType> mailbox_;

        Router router_;

        ActorHandle actor_;
    };

    template<class T>
    requires std::is_base_of_v<Message, T>
    ActorContextRouter<T>::ActorContextRouter(ActorContext &ctx)
        : ctx_(ctx) {
    }

    template<class T>
    requires std::is_base_of_v<Message, T>
    ActorContextRouter<T>::~ActorContextRouter() {

    }

    template<class T>
    requires std::is_base_of_v<Message, T>
    ActorContext &ActorContextRouter<T>::getContext() const {
        return ctx_;
    }

    template<class Router>
    requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
    ActorContextImpl<Router>::ActorContextImpl(asio::io_context &ctx)
        : ctx_(ctx),
          mailbox_(ctx_, 1024) {

    }

    template<class Router>
    requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
    ActorContextImpl<Router>::~ActorContextImpl() {

    }

    template<class Router>
    requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
    asio::io_context &ActorContextImpl<Router>::getIOContext() const {
        return ctx_;
    }

    template<class Router>
    requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
    Router &ActorContextImpl<Router>::getRouter() {
        return router_;
    }

    template<class Router>
    requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
    void ActorContextImpl<Router>::setActor(ActorHandle &&handle) {
        if (actor_ != nullptr)
            return;

        if (!mailbox_.is_open())
            return;

        actor_ = std::move(handle);
    }

    template<class Router>
    requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
    Actor *ActorContextImpl<Router>::getActor() const {
        return actor_.get();
    }

    template<class Router>
    requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
    void ActorContextImpl<Router>::sendMessage(MessageHandle &&msg) {
        router_.sendMessage(std::move(msg));
    }

    template<class Router>
    requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
    void ActorContextImpl<Router>::sendMessage(Message *msg) {
        sendMessage(MessageHandleType{msg, Message::Deleter::make()});
    }
}
