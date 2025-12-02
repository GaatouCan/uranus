#include <spdlog/spdlog.h>

#include "gateway/PackageCodec.h"

using uranus::network::ConnectionImpl;
using uranus::PackageCodec;

int main() {
    spdlog::info("Hello World!");

    asio::io_context ctx;
    asio::ssl::context sslctx(asio::ssl::context::tls_server);
    uranus::network::TcpSocket socket(ctx, sslctx);

    auto conn = new ConnectionImpl<PackageCodec>(std::move(socket));
    delete conn;

    return 0;
}