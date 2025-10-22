#pragma once

#include "Common.h"
#include "base/Types.h"

#include <memory>

namespace uranus {

    using std::error_code;
    using std::unique_ptr;
    using std::make_unique;
    using std::shared_ptr;
    using std::make_shared;

    class DataAsset;
    class GameServer;
    class AbstractActor;
    class ChannelNode;
    struct Message;


    class CORE_API ActorContext : public std::enable_shared_from_this<ActorContext> {

        struct SessionNode {
            std::function<void(Message *)> handler;
            asio::executor_work_guard<asio::any_io_executor> work;
        };

    // protected:
    //     using ActorChannel = default_token::as_default_on_t<asio::experimental::concurrent_channel<void(error_code, ChannelNode *)>>;

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

        template<asio::completion_token_for<void(Message *)> CompleteToken>
        auto AsyncCall(int64_t target, Message *req, CompleteToken &&token);

        virtual void PushMessage(Message *msg) = 0;

    protected:
        void SetUpActor();

        [[nodiscard]] bool IsChannelClosed() const;

        void PushNode(unique_ptr<ChannelNode> &&node) const;

        awaitable<void> Process();

        virtual void CleanUp();

    private:
        /// The pointer to the Game Server
        GameServer *const server_;

        /// The reference to the io_context
        asio::io_context &ctx_;

    protected:
        /// The inner channel to handle the tasks
        unique_ptr<ConcurrentChannel<unique_ptr<ChannelNode>>> channel_;
    };

    template<asio::completion_token_for<void(Message *)> CompleteToken>
    auto ActorContext::AsyncCall(int64_t target, Message *req, CompleteToken &&token) {
        auto init = [](asio::completion_handler_for<void(Message *)> auto handler, Message *req) {
            auto work = asio::make_work_guard(handler);
            auto node = make_unique<SessionNode>(std::move(handler), std::move(work));
        };

        return asio::async_initiate<CompleteToken, void(Message *)>(init, token, req);
    }
}
