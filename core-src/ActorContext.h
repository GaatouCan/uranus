#pragma once

#include "Message.h"
#include "base/Types.h"
#include "base/IdentAllocator.h"

#include <memory>
#include <shared_mutex>
#include <unordered_map>

namespace uranus {

    using std::error_code;
    using std::unique_ptr;
    using std::shared_ptr;
    using std::make_shared;
    using std::make_unique;
    using std::shared_mutex;
    using std::shared_lock;
    using std::unique_lock;
    using std::optional;

    class DataAsset;
    class GameServer;
    class AbstractActor;
    class ChannelNode;

    class CORE_API ActorContext : public std::enable_shared_from_this<ActorContext> {

    protected:
        struct SessionNode {
            std::function<void(optional<Message>)> handler;
            asio::executor_work_guard<asio::any_io_executor> work;
        };

    public:
        ActorContext() = delete;

        explicit ActorContext(GameServer *ser);
        virtual ~ActorContext();

        DISABLE_COPY_MOVE(ActorContext)

        [[nodiscard]] GameServer *GetGameServer() const;

        asio::io_context &GetIOContext();

        [[nodiscard]] virtual AbstractActor *GetActor() const = 0;

        virtual int Initial(DataAsset *data);
        virtual int Start();
        virtual void Stop();

        /// Return a message with custom resource
        [[nodiscard]] virtual Message BuildMessage() = 0;

        /// Send the message to the other actor;
        /// You should set the .type in msg to tell what kind of actor you want to send
        virtual void Send(int64_t target, const Message &msg) = 0;

        /// Send the message to the service actor by target service' s name
        virtual void SendToService(const std::string &name, const Message &msg) = 0;

        /// Send a request type message to the target service
        /// It will be suspended till receive the response
        template<asio::completion_token_for<void(optional<Message>)> CompleteToken>
        auto AsyncCall(int64_t target, const Message &req, CompleteToken &&token);

        /// Other push message to this context;
        /// Do not call this method in logic code
        void PushMessage(const Message &msg);

    protected:
        /// Make actor' s context pointer point to this context
        void SetUpActor();

        [[nodiscard]] bool IsChannelClosed() const;

        /// Implement how to send the request message to the target actor
        virtual void RemoteCall(int64_t target, Message msg, SessionNode &&node) = 0;

        int32_t AllocateSessionID();
        void RecycleSessionID(int32_t id);

        void PushSession(int32_t id, SessionNode &&node);
        std::optional<SessionNode> TakeSession(int32_t id);

        awaitable<void> Process();

        /// Handle the message that this context received
        /// This method will be called in ::Process automatically
        virtual void HandleMessage(const Message &msg) = 0;

        /// Implement how to release the custom resource in message
        virtual void DisposeMessage(const Message &msg) = 0;

        /// Clean Up the resource in this context after ::Stop be called
        /// This method will be called in the end of ::Process automatically
        virtual void CleanUp();

    private:
        /// The pointer to the Game Server
        GameServer *const server_;

        /// The reference to the io_context
        asio::io_context &ctx_;

    protected:
        /// The inner channel to handle the tasks
        unique_ptr<ConcurrentChannel<Message>> channel_;

        /// Allocate and recycle the session ids
        IdentAllocator<int32_t, false> sess_id_alloc_;

        /// The mapping to all the spending request
        std::unordered_map<int32_t, SessionNode> sessions_;
    };

    template<asio::completion_token_for<void(optional<Message>)> CompleteToken>
    auto ActorContext::AsyncCall(int64_t target, const Message &req, CompleteToken &&token) {
        auto init = [this, target](asio::completion_handler_for<void(optional<Message>)> auto handler, const Message &request) {
            auto work = asio::make_work_guard(handler);
            this->RemoteCall(target, request, SessionNode{std::move(handler), std::move(work)});
        };

        return asio::async_initiate<CompleteToken, void(optional<Message>)>(init, token, req);
    }
}
