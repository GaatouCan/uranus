#include <spdlog/spdlog.h>

#include "gateway/PackageCodec.h"
#include "gateway/GameWorldHandler.h"

using uranus::network::ConnectionImpl;
using uranus::PackageCodec;
using uranus::GameWorldHandler;

int main() {
    spdlog::info("Hello World!");

    asio::io_context ctx;
    asio::ssl::context sslctx(asio::ssl::context::tls_server);
    uranus::network::TcpSocket socket(ctx, sslctx);

    auto conn = std::make_shared<ConnectionImpl<PackageCodec, GameWorldHandler>>(std::move(socket));

    return 0;
}