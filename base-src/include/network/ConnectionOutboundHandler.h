#pragma once

#include "ConnectionHandler.h"
#include "base/Message.h"

#include <asio/awaitable.hpp>

namespace uranus::network {

    class ConnectionPipelineContext;

    using asio::awaitable;
    using std::error_code;
    using std::exception;

    class BASE_API ConnectionOutboundHandler : virtual public ConnectionHandler {

    public:
        ConnectionOutboundHandler();
        ~ConnectionOutboundHandler() override;

        [[nodiscard]] HandlerType type() const override;

        virtual awaitable<void> beforeSend(ConnectionPipelineContext &ctx, Message *msg);
        virtual awaitable<void> afterSend(ConnectionPipelineContext &ctx, MessageHandle &&msg);
    };
}