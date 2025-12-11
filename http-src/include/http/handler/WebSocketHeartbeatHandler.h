#pragma once

#include "../WebSocketFrame.h"

#include <network/ConnectionDeluxeHandler.h>
#include <network/Types.h>
#include <base/Types.h>


namespace uranus::http {

    using network::ConnectionDeluxeHandler;
    using network::ConnectionPipelineContext;
    using asio::awaitable;

    /**
     * 处理WebSocket中的Ping/Pong/Close帧
     */
    class HTTP_API WebSocketHeartbeatHandler final : public ConnectionDeluxeHandler {

    public:
        /// pingSecond 大于0表示隔多少秒主动发送Ping帧
        explicit WebSocketHeartbeatHandler(int32_t pingSecond = -1);

        // awaitable<void> onReceiveT(ConnectionPipelineContext &ctx, MessageUnique &&frame) override;

        awaitable<void> onReceive(ConnectionPipelineContext &ctx, MessageHandle &&msg) override;
        void afterWrite(ConnectionPipelineContext &ctx, Message *msg) override;

    private:
        awaitable<void> pingToClient();

    private:
        int32_t pingSec_;
        SteadyTimePoint lastPong_;

        bool closeByServer_;
    };
}
