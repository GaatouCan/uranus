#include "Gateway.h"
#include "../GameWorld.h"

Gateway::Gateway(ServerBootstrap &server)
    : ServerModule(server),
      sslContext_(asio::ssl::context::tlsv13_server),
      acceptor_(ctx_) {
}

Gateway::~Gateway() {
}

GameWorld &Gateway::getWorld() const {
    return dynamic_cast<GameWorld &>(getServer());
}

void Gateway::start() {
    sslContext_.use_certificate_chain_file("server.crt");
    sslContext_.use_private_key_file("server.key", asio::ssl::context::pem);
    sslContext_.set_options(
        asio::ssl::context::no_sslv2 |
        asio::ssl::context::no_sslv3 |
        asio::ssl::context::default_workarounds |
        asio::ssl::context::single_dh_use
    );
}

void Gateway::stop() {
}

awaitable<void> Gateway::waitForClient(uint16_t port) {
    try {
        acceptor_.open(asio::ip::tcp::v4());
        acceptor_.bind({asio::ip::tcp::v4(), port});
        acceptor_.listen(port);

        while (!ctx_.stopped()) {
            auto [ec, socket] = co_await acceptor_.async_accept(pool_.getIOContext());
            if (ec) {
                // TODO
                continue;
            }

            if (!socket.is_open()) {
                continue;
            }

            auto conn = uranus::MakeConnection<PackageCodec, GatewayHandler>(TcpSocket(std::move(socket), sslContext_));

            conn->getHandler().setGateway(this);
        }
    } catch (std::exception &e) {

    }
}
