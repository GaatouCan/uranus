#pragma once

#include "BaseActor.h"

#include <base/Message.h>
#include <base/types.h>
#include <base/AttributeMap.h>
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

    class ACTOR_API ActorContext {

    public:
        ActorContext() = delete;

        explicit ActorContext(asio::io_context &ctx);
        virtual ~ActorContext();

        DISABLE_COPY_MOVE(ActorContext)

        void setActor(ActorHandle &&handle);
        [[nodiscard]] BaseActor *getActor() const;

        void setId(uint32_t id);
        [[nodiscard]] uint32_t getId() const;

        [[nodiscard]] asio::io_context &getIOContext() const;

        [[nodiscard]] bool isRunning() const;

        AttributeMap &attr();

        virtual void run() = 0;
        virtual void terminate() = 0;

        void pushEnvelope(Envelope &&envelope);

        virtual void sendMessage(int32_t ty, uint32_t target, MessageHandle &&msg) = 0;
        virtual void sendMessage(int32_t ty, uint32_t target, Message *msg) = 0;

    protected:
        asio::io_context &ctx_;
        ConcurrentChannel<Envelope> mailbox_;

        ActorHandle actor_;
        AttributeMap attr_;

        uint32_t id_;
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
        [[nodiscard]] BaseActor *getActor() const;

        virtual void onInitial() {};
        virtual void onTerminate() {};

        virtual void onMessage(int32_t type, uint32_t src, Type *msg) {}
        virtual void sendMessage(int32_t ty, uint32_t target, HandleType &&msg) = 0;

        virtual void onError(error_code ec) {};
        virtual void onException(const std::exception &e) {};

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

            explicit ActorContextImpl(asio::io_context &ctx);
            ~ActorContextImpl() override;

            DISABLE_COPY_MOVE(ActorContextImpl)

            Router &getRouter();

            void run() override;
            void terminate() override;

            void sendMessage(int32_t ty, uint32_t target, MessageHandle &&msg) override;
            void sendMessage(int32_t ty, uint32_t target, Message *msg) override;

        private:
            awaitable<void> processMessage();

        private:
            Router router_;
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

    template<class T> requires std::is_base_of_v<Message, T>
    BaseActor *ActorContextRouter<T>::getActor() const {
        return ctx_.getActor();
    }

    namespace detail {

        template<class Router>
        requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
        ActorContextImpl<Router>::ActorContextImpl(asio::io_context &ctx)
            : ActorContext(ctx),
              router_(*this) {

        }

        template<class Router>
        requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
        ActorContextImpl<Router>::~ActorContextImpl() {
            ActorContextImpl::terminate();
        }

        template<class Router>
        requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
        Router &ActorContextImpl<Router>::getRouter() {
            return router_;
        }

        template<class Router>
        requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
        void ActorContextImpl<Router>::run() {
            if (actor_ == nullptr) {
                // TODO
                return;
            }

            router_.onInitial();

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
        void ActorContextImpl<Router>::sendMessage(int32_t ty, uint32_t target, MessageHandle &&msg) {
            if (msg == nullptr)
                return;

            auto del = msg.get_deleter();
            auto *ptr = msg.get();

            if (auto *temp = dynamic_cast<MessageType *>(ptr)) {
                MessageHandleType handle{ temp, del };
                router_.sendMessage(ty, target, std::move(handle));
                return;
            }

            del(ptr);
        }

        template<class Router>
        requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
        void ActorContextImpl<Router>::sendMessage(int32_t ty, uint32_t target, Message *msg) {
            sendMessage(ty, target, MessageHandle{ msg, Message::Deleter::make() });
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
                        router_.onError(ec);
                        continue;
                    }

                    router_.onMessage(envelope.type, envelope.source, dynamic_cast<MessageType *>(envelope.message.get()));
                    actor_->onMessage(std::move(envelope));
                }

                router_.onTerminate();
            } catch (const std::exception &e) {
                router_.onException(e);
            }
        }
    }

    template<class Router>
    requires std::is_base_of_v<ActorContextRouter<typename Router::Type>, Router>
    shared_ptr<detail::ActorContextImpl<Router>> MakeActorContext(asio::io_context &ctx) {
        return make_shared<detail::ActorContextImpl<Router>>(ctx);
    }
}
