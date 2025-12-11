#include "handler/WebSocketHeartbeatHandler.h"

#include <network/ConnectionPipelineContext.h>
#include <spdlog/spdlog.h>


namespace uranus::http {

    using network::SteadyTimer;

    WebSocketHeartbeatHandler::WebSocketHeartbeatHandler(const int32_t pingSecond)
        : pingSec_(pingSecond),
          closeByServer_(false) {
        if (pingSec_ > 0) {
            asio::co_spawn(getContext().getExecutor(), pingToClient(), asio::detached);
        }
    }

    awaitable<void> WebSocketHeartbeatHandler::onReceive(ConnectionPipelineContext &ctx, MessageHandle &&msg) {
        const auto *frame = dynamic_cast<WebSocketFrame *>(msg.get());
        if (frame == nullptr) {
            co_await ctx.fireReceive(std::move(msg));
            co_return;
        }

        if (frame->getOpcode() != 0x08 &&
            frame->getOpcode() != 0x09 &&
            frame->getOpcode() != 0x0A) {
            co_await ctx.fireReceive(std::move(msg));
            co_return;
        }

        if (frame->getOpcode() == 0x08) {
            // 检查到主动发送之后
            // 收到这个Close就直接关闭
            if (closeByServer_) {
                ctx.getConnection().disconnect();
                co_return;
            }

            SPDLOG_INFO("Connection[{}] - Receive close operation from client", ctx.getConnection().getKey());

            auto res = WebSocketFrame::getUnique();
            res->setOpcode(0x08);
            ctx.send(std::move(res));
            co_return;
        }

        // 如果是客户端发来的Ping帧
        if (frame->getOpcode() == 0x09) {
            auto res = WebSocketFrame::getUnique();
            res->setOpcode(0x0A);
            res->setData(frame->getPayloadAsStringView());
            ctx.send(std::move(res));

            co_return;
        }

        // 如果收到Pong帧可以先不管它
        if (frame->getOpcode() == 0x0A) {
            lastPong_ = std::chrono::steady_clock::now();
        }
    }

    void WebSocketHeartbeatHandler::afterWrite(ConnectionPipelineContext &ctx, Message *msg) {
        const auto *frame = dynamic_cast<WebSocketFrame *>(msg);

        if (frame == nullptr) {
            ctx.fireAfterWrite(msg);
            return;
        }

        if (frame->getOpcode() == 0x08) {
            // 如果是客户端发起的关闭连接
            // 那么服务端发送完之后就可以断开链接了
            if (!closeByServer_) {
                SPDLOG_INFO("Connection[{}] - Close after send close opcode", ctx.getConnection().getKey());
                ctx.getConnection().disconnect();
                return;
            }

            // 不然就是服务端主动发起断开链接
            closeByServer_ = true;
            SPDLOG_INFO("Connection[{}] - WebSocket closed by server", ctx.getConnection().getKey());
        }
    }


    awaitable<void> WebSocketHeartbeatHandler::pingToClient() {
        if (pingSec_ <= 0)
            co_return;

        try {
            lastPong_ = std::chrono::steady_clock::now();

            const auto executor = co_await asio::this_coro::executor;
            SteadyTimer timer(executor);

            while (true) {

                static constexpr auto kPingPayload = "Heartbeat";

                timer.expires_after(std::chrono::seconds(pingSec_));
                const auto [ec] = co_await timer.async_wait();
                if (ec)
                    break;

                const auto now = std::chrono::steady_clock::now();

                // 如果上一个Ping包还没有回应
                if (now - lastPong_ > std::chrono::seconds(pingSec_)) {
                    getContext().getConnection().disconnect();
                    break;
                }

                auto ping = WebSocketFrame::getUnique();

                ping->setOpcode(0x09);
                ping->setData(std::string_view(kPingPayload));

                getContext().send(std::move(ping));
            }
        } catch (std::exception &e) {
            SPDLOG_WARN("WebSocket ping error: {}", e.what());
        }
    }
}
