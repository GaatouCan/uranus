#pragma once

#include "base/Message.h"
#include "ConnectionHandler.h"

#include <asio/awaitable.hpp>

namespace uranus::network {

    class ConnectionPipelineContext;

    using asio::awaitable;
    using std::error_code;
    using std::exception;

    class BASE_API ConnectionInboundHandler : virtual public ConnectionHandler {

    public:
        ConnectionInboundHandler();
        ~ConnectionInboundHandler() override;

        [[nodiscard]] HandlerType type() const override;

        virtual void onConnect(ConnectionPipelineContext &ctx);
        virtual void onDisconnect(ConnectionPipelineContext &ctx);
        virtual void onError(ConnectionPipelineContext &ctx, error_code ec);
        virtual void onException(ConnectionPipelineContext &ctx, exception &e);
        virtual void onTimeout(ConnectionPipelineContext &ctx);

        virtual awaitable<void> onReceive(ConnectionPipelineContext &ctx, MessageHandle &&msg) = 0;
    };
}
