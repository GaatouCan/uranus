#pragma once

#include "../HttpRequest.h"

#include <network/SimpleInboundHandler.h>

namespace uranus::http {

    using network::SimpleInboundHandler;
    using network::ConnectionPipelineContext;
    using asio::awaitable;

    class HTTP_API WebSocketUpgradeHandler final : public SimpleInboundHandler<HttpRequest> {
    public:
        awaitable<void> onReceiveT(ConnectionPipelineContext &ctx, MessageUnique &&req) override;
    };
}
