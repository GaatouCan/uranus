#include "handler/HttpServerHandler.h"
#include "HttpServerCodec.h"
#include "HttpResponse.h"
#include "HttpUtils.h"

namespace uranus::http {

    awaitable<void> HttpServerHandler::onReceiveT(
        ConnectionPipelineContext &ctx,
        MessageUnique &&req)
    {
        if (req->getHeader("Expect") == "100-continue") {
            // TODO
        }

        auto &codec = ctx.getCodec<HttpServerCodec>();

        const auto te = req->getHeader("Transfer-Encoding");
        const auto cl = req->getHeaderValueAsULongLong("Content-Length");

        if (!te.empty() && detail::case_ignore::equal(te, "chunked")) {
            co_await codec.readChunked(req.get(), [&](std::string_view sv) {
                // 默认添加到Body尾部 可以改成落盘处理
                req->body_.append(sv);
            });
        } else if (cl > 0) {
            co_await codec.readContent(req.get(), [&](std::string_view sv) {
                req->body_.append(sv);
            });
        }

        co_await ctx.fireReceive(std::move(req));
    }
}
