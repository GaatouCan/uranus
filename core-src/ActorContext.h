#pragma once

#include "Common.h"
#include "base/Types.h"

#include <memory>
#include <asio/experimental/concurrent_channel.hpp>


using std::error_code;
using std::unique_ptr;
using std::make_unique;
using std::shared_ptr;
using std::make_shared;


class GameServer;
class AbstractActor;


class CORE_API ActorContext : public std::enable_shared_from_this<ActorContext> {

protected:
#pragma region Channel Node

    class CORE_API ChannelNode {

    public:
        ChannelNode() = default;
        virtual ~ChannelNode() = default;

        DISABLE_COPY_MOVE(ChannelNode)

        virtual void Execute(ActorContext *ctx) = 0;
        virtual void CleanUp(ActorContext *ctx) {}
    };

#pragma endregion

    using ActorChannel = default_token::as_default_on_t<asio::experimental::concurrent_channel<void(error_code, unique_ptr<ChannelNode>)>>;

public:
    ActorContext() = delete;

    explicit ActorContext(GameServer *ser);
    virtual ~ActorContext();

    DISABLE_COPY_MOVE(ActorContext)

    [[nodiscard]] GameServer *GetGameServer() const;

    asio::io_context &GetIOContext();

    [[nodiscard]] virtual AbstractActor *GetActor() const = 0;

protected:
    void SetUpActor();

    awaitable<void> Process();

    virtual void CleanUp();

private:
    GameServer *const server_;
    asio::io_context &ctx_;

    ActorChannel channel_;
};
