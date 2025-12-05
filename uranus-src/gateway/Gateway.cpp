#include "Gateway.h"
#include "../GameWorld.h"

#include <spdlog/spdlog.h>
#include <asio/signal_set.hpp>

Gateway::Gateway(GameWorld &world)
    : ServerModule(world),
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

    pool_.start();

    SPDLOG_INFO("Listening on port: {}", 8080);
    co_spawn(ctx_, waitForClient(8080), detached);

    asio::signal_set signals(ctx_, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto) {
        stop();
    });

    ctx_.run();
}

void Gateway::stop() {
    if (!ctx_.stopped()) {
        ctx_.stop();
    }

    pool_.stop();
}

Gateway::ConnectionPointer Gateway::find(const std::string &key) const {
    if (ctx_.stopped())
        return nullptr;

    std::shared_lock lock(mutex_);
    const auto it = connMap_.find(key);
    return it == connMap_.end() ? nullptr : it->second;
}

void Gateway::removeConnection(const std::string &key) {
    if (ctx_.stopped())
        return;

    std::unique_lock lock(mutex_);
    connMap_.erase(key);
}

awaitable<void> Gateway::waitForClient(uint16_t port) {
    try {
        acceptor_.open(asio::ip::tcp::v4());
        acceptor_.bind({asio::ip::tcp::v4(), port});
        acceptor_.listen(port);

        SPDLOG_INFO("Waiting for client...");

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
            SPDLOG_INFO("New connection from: {}", conn->remoteAddress().to_string());

            // Initial connection
            {
                conn->setExpirationSecond(30);
                conn->getHandler().setGateway(this);
            }

            bool repeated = false;

            do {
                std::unique_lock lock(mutex_);

                if (connMap_.contains(conn->getKey())) {
                    repeated = true;
                    break;
                }

                connMap_.insert_or_assign(conn->getKey(), conn);
            } while (false);

            if (repeated) {
                SPDLOG_WARN("Connection key repeated! from {}", conn->remoteAddress().to_string());
                conn->attr().set("REPEATED", true);
                conn->disconnect();
                continue;
            }

            SPDLOG_INFO("Accepted new connection from: {}", conn->remoteAddress().to_string());
            conn->connect();
        }
    } catch (std::exception &e) {
        SPDLOG_ERROR("{}", e.what());
    }
}
