#pragma once

#include "../HttpRequest.h"

#include <network/SimpleInboundHandler.h>


namespace uranus::http {

    using network::ConnectionPipelineContext;
    using network::SimpleInboundHandler;
    using asio::awaitable;

    class HTTP_API HttpServerHandler final : public SimpleInboundHandler<HttpRequest> {

    public:
        awaitable<void> onReceiveT(ConnectionPipelineContext &ctx, MessageUnique &&req) override;
    };
}