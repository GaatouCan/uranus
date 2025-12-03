#pragma once

#include "BaseActor.h"

#include <base/Message.h>
#include <base/types.h>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>


namespace uranus::actor {

    using asio::awaitable;
    using asio::co_spawn;
    using asio::detached;

    using std::tuple;
    using std::error_code;
    using std::shared_ptr;
    using std::make_shared;
    using std::enable_shared_from_this;

    class ActorContext {

    public:
        ActorContext() = default;
        virtual ~ActorContext() = default;

        DISABLE_COPY_MOVE(ActorContext)

        virtual void setActor(ActorHandle &&handle) = 0;
        [[nodiscard]] virtual BaseActor *getActor() const = 0;

        virtual void setId(uint32_t id) = 0;
        [[nodiscard]] virtual uint32_t getId() const = 0;

        virtual void run() = 0;
        virtual void terminate() = 0;

        virtual void pushEnvelope(Envelope &&envelope) = 0;

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

    namespace detail {

        template<class Router>
        requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
        class ActorContextImpl final : public ActorContext, public enable_shared_from_this<ActorContextImpl<Router>> {

        public:
            using MessageType = Router::Type;
            using MessageHandleType = Router::HandleType;

            ActorContextImpl() = delete;

            explicit ActorContextImpl(asio::io_context &ctx);
            ~ActorContextImpl() override;

            DISABLE_COPY_MOVE(ActorContextImpl)

            [[nodiscard]] asio::io_context &getIOContext() const;

            Router &getRouter();

            void setActor(ActorHandle &&handle) override;
            [[nodiscard]] BaseActor *getActor() const override;

            void setId(uint32_t id) override;
            [[nodiscard]] uint32_t getId() const override;

            void run() override;
            void terminate() override;

            [[nodiscard]] bool isRunning() const;

            void pushEnvelope(Envelope &&envelope) override;

            void sendMessage(MessageHandle &&msg) override;
            void sendMessage(Message *msg) override;

        private:
            awaitable<void> processMessage();

        private:
            asio::io_context &ctx_;
            ConcurrentChannel<Envelope> mailbox_;

            Router router_;

            ActorHandle actor_;

            uint32_t id_;
        };
    }

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

    namespace detail {

        template<class Router>
        requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
        ActorContextImpl<Router>::ActorContextImpl(asio::io_context &ctx)
            : ctx_(ctx),
              mailbox_(ctx_, 1024),
              actor_(nullptr),
              id_(0) {

        }

        template<class Router>
        requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
        ActorContextImpl<Router>::~ActorContextImpl() {
            ActorContextImpl::terminate();
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
            actor_->setContext(this);
        }

        template<class Router>
        requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
        BaseActor *ActorContextImpl<Router>::getActor() const {
            return actor_.get();
        }

        template<class Router>
        requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
        void ActorContextImpl<Router>::setId(const uint32_t id) {
            id_ = id;
        }

        template<class Router>
        requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
        uint32_t ActorContextImpl<Router>::getId() const {
            return id_;
        }

        template<class Router>
        requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
        void ActorContextImpl<Router>::run() {
            if (actor_ == nullptr) {
                // TODO
                return;
            }

            // FIXME: actor::start()

            co_spawn(ctx_, [self = this->shared_from_this()]() -> awaitable<void> {
                co_await self->processMessage();
            }, detached);
        }

        template<class Router>
        requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
        void ActorContextImpl<Router>::terminate() {
            if (!mailbox_.is_open())
                return;

            mailbox_.cancel();
            mailbox_.close();
        }

        template<class Router>
        requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
        bool ActorContextImpl<Router>::isRunning() const {
            return actor_ != nullptr && mailbox_.is_open();
        }

        template<class Router>
        requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
        void ActorContextImpl<Router>::pushEnvelope(Envelope &&envelope) {
            if (!mailbox_.is_open())
                return;

            if (actor_ == nullptr)
                return;

            mailbox_.try_send_via_dispatch(error_code{}, std::move(envelope));
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

        template<class Router>
        requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
        awaitable<void> ActorContextImpl<Router>::processMessage() {
            try {
                while (mailbox_.is_open()) {
                    auto [ec, envelope] = co_await mailbox_.async_receive();

                    if (ec == asio::error::operation_aborted ||
                        ec == asio::experimental::error::channel_closed) {
                        // TODO
                        break;
                        }

                    if (ec) {
                        // TODO
                        continue;
                    }

                    actor_->onMessage(std::move(envelope));
                }
            } catch (const std::exception &e) {

            }
        }
    }

    template<class Router>
    requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
    shared_ptr<detail::ActorContextImpl<Router>> MakeActorContext(asio::io_context &ctx) {
        return make_shared<detail::ActorContextImpl<Router>>(ctx);
    }
}
