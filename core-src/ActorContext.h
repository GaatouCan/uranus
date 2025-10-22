#pragma once

#include "Common.h"
#include "base/Types.h"
#include "base/IdentAllocator.h"

#include <memory>
#include <shared_mutex>
#include <unordered_map>

namespace uranus {

    using std::error_code;
    using std::unique_ptr;
    using std::make_unique;
    using std::shared_ptr;
    using std::make_shared;
    using std::shared_mutex;
    using std::shared_lock;
    using std::unique_lock;

    class DataAsset;
    class GameServer;
    class AbstractActor;
    class ChannelNode;
    struct Message;


    class CORE_API ActorContext : public std::enable_shared_from_this<ActorContext> {

    protected:
        struct SessionNode {
            std::function<void(Message *)> handler;
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

        [[nodiscard]] virtual Message *BuildMessage() = 0;

        virtual void Send(int64_t target, Message *msg) = 0;

        virtual void SendToService(const std::string &name, Message *msg) = 0;

        virtual void RemoteCall(int64_t target, Message *msg, unique_ptr<SessionNode> &&node) = 0;

        template<asio::completion_token_for<void(Message *)> CompleteToken>
        auto AsyncCall(int64_t target, Message *req, CompleteToken &&token);

        virtual void PushMessage(Message *msg) = 0;

    protected:
        void SetUpActor();

        [[nodiscard]] bool IsChannelClosed() const;

        void PushNode(unique_ptr<ChannelNode> &&node) const;

        int32_t AllocateSessionID();
        void RecycleSessionID(int32_t id);

        void PushSession(int32_t id, unique_ptr<SessionNode> &&node);
        unique_ptr<SessionNode> TakeSession(int32_t id);

        awaitable<void> Process();

        virtual void OnResponse(Message *msg) = 0;

        virtual void CleanUp();

    private:
        /// The pointer to the Game Server
        GameServer *const server_;

        /// The reference to the io_context
        asio::io_context &ctx_;

    protected:
        /// The inner channel to handle the tasks
        unique_ptr<ConcurrentChannel<unique_ptr<ChannelNode>>> channel_;

        IdentAllocator<int32_t, true> sess_id_alloc_;

        mutable shared_mutex sess_mutex_;
        std::unordered_map<int32_t, unique_ptr<SessionNode>> sessions_;
    };

    template<asio::completion_token_for<void(Message *)> CompleteToken>
    auto ActorContext::AsyncCall(int64_t target, Message *req, CompleteToken &&token) {
        auto init = [this, target](asio::completion_handler_for<void(Message *)> auto handler, Message *request) {
            auto work = asio::make_work_guard(handler);
            auto node = make_unique<SessionNode>(std::move(handler), std::move(work));
            this->RemoteCall(target, request, std::move(node));
        };

        return asio::async_initiate<CompleteToken, void(Message *)>(init, token, req);
    }
}
