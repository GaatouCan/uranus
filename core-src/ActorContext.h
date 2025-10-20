#pragma once

#include "Common.h"
#include "base/Types.h"

#include <memory>
#include <asio/experimental/concurrent_channel.hpp>


namespace uranus {

    using std::error_code;
    using std::unique_ptr;
    using std::make_unique;
    using std::shared_ptr;
    using std::make_shared;

    class GameServer;
    class AbstractActor;
    class ChannelNode;
    class Message;


    class CORE_API ActorContext : public std::enable_shared_from_this<ActorContext> {

        using ActorChannel = default_token::as_default_on_t<asio::experimental::concurrent_channel<void(error_code, unique_ptr<ChannelNode>)>>;

    public:
        ActorContext() = delete;

        explicit ActorContext(GameServer *ser);
        virtual ~ActorContext();

        DISABLE_COPY_MOVE(ActorContext)

        [[nodiscard]] GameServer *GetGameServer() const;

        asio::io_context &GetIOContext();

        [[nodiscard]] virtual AbstractActor *GetActor() const = 0;

        virtual void SendToService(int64_t target, Message *msg) = 0;
        virtual void SendToService(const std::string &name, Message *msg) = 0;

        virtual void SendToPlayer(int64_t pid, Message *msg) = 0;
        virtual void SendToClient(int64_t pid, Message *msg) = 0;

        virtual void PushPackage(Message *pkg) = 0;

    protected:
        void SetUpActor();

        void PushNode(unique_ptr<ChannelNode> &&node);

        template<class T, class... Args>
        requires std::is_base_of_v<ChannelNode, T>
        void EmplaceNode(Args &&... args);

        awaitable<void> Process();

        virtual void CleanUp();

    private:
        GameServer *const server_;
        asio::io_context &ctx_;

        ActorChannel channel_;
    };

    template<class T, class ... Args>
    requires std::is_base_of_v<ChannelNode, T>
    void ActorContext::EmplaceNode(Args &&...args) {
        auto node = make_unique<ChannelNode>(std::forward<Args>(args)...);
        this->PushNode(std::move(node));
    }
}
