#include "handler/WebSocketUpgradeHandler.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpServerCodec.h"
#include "HttpUtils.h"

#include <base/utils.h>
#include <network/ConnectionPipelineContext.h>
#include <spdlog/spdlog.h>


namespace uranus::http {

    static constexpr auto kGuid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    awaitable<void> WebSocketUpgradeHandler::onReceiveT(
        ConnectionPipelineContext &ctx,
        MessageUnique &&req)
    {
        if (req == nullptr)
            co_return;

        if (req->getMethod() != kGet) {
            co_await ctx.fireReceive(std::move(req));
            co_return;
        }

        const auto connection = req->getHeader("Connection");
        const auto upgrade = req->getHeader("Upgrade");
        const auto wsKey = req->getHeader("Sec-WebSocket-Key");

        if (connection.empty() || upgrade.empty() || wsKey.empty()) {
            co_await ctx.fireReceive(std::move(req));
            co_return;
        }

        if (!(detail::case_ignore::contains(connection, "upgrade") &&
            detail::case_ignore::equal(upgrade, "websocket"))) {
            co_await ctx.fireReceive(std::move(req));
            co_return;
        }

        const std::string src = wsKey + kGuid;
        const auto digest = utils::crypto::Sha1Calculate(src);
        const std::string accept = utils::crypto::Base64Encode(std::string_view(reinterpret_cast<const char*>(digest.data()), digest.size()));

        auto res = HttpResponse::getUnique();

        res->setStatus(StatusCode::kSwitchingProtocol);
        res->setHeader("Upgrade", "websocket");
        res->setHeader("Connection", "Upgrade");
        res->setHeader("Sec-WebSocket-Accept", accept);

        // 写回 101 响应
        ctx.send(std::move(res));

        // 切换到 WebSocket 模式
        auto &codec = ctx.getCodec<HttpServerCodec>();
        codec.switchToWebSocket();

        SPDLOG_INFO("Connection[{}] - Upgrade to WebSocket", ctx.getConnection().getKey());
    }
}
