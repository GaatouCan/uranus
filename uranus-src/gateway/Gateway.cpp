#include "Gateway.h"
#include "ActorConnection.h"

#include <asio/signal_set.hpp>
#include <spdlog/spdlog.h>


Gateway::Gateway(GameWorld &world)
    : world_(world),
      guard_(asio::make_work_guard(ctx_)),
      acceptor_(ctx_)
#ifdef URANUS_SSL
      , sslContext_(asio::ssl::context::tlsv13_server)
#endif
{
}

Gateway::~Gateway() {
    if (thread_.joinable()) {
        thread_.join();
    }
}

void Gateway::start() {
#ifdef URANUS_SSL
    sslContext_.set_options(
        asio::ssl::context::no_sslv2 |
        asio::ssl::context::no_sslv3 |
        asio::ssl::context::default_workarounds |
        asio::ssl::context::single_dh_use
    );

    sslContext_.use_certificate_chain_file("server.crt");
    sslContext_.use_private_key_file("server.pem", asio::ssl::context::pem);
#endif

    co_spawn(ctx_, waitForClient(8080), detached);

    thread_ = std::thread([this] {
        asio::signal_set signals(ctx_, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) {
            stop();
        });

        ctx_.run();
    });
}

void Gateway::stop() {
    if (ctx_.stopped())
        return;

    guard_.reset();
    ctx_.stop();

    connMap_.clear();
}

std::shared_ptr<ActorConnection> Gateway::find(const std::string &key) const {
    if (ctx_.stopped())
        return nullptr;

    std::shared_lock lock(mutex_);
    const auto it = connMap_.find(key);
    return it != connMap_.end() ? it->second : nullptr;
}

std::shared_ptr<ActorConnection> Gateway::findByPlayerID(const uint32_t pid) const {
    if (ctx_.stopped())
        return nullptr;

    std::shared_lock lock(mutex_);
    const auto it = pidToKey_.find(pid);
    if (it == pidToKey_.end()) {
        return nullptr;
    }

    const auto key = it->second;
    const auto iter = connMap_.find(key);
    return iter != connMap_.end() ? iter->second : nullptr;
}

void Gateway::remove(const std::string &key) {
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

        SPDLOG_INFO("Listening on port: {}", port);

        while (!ctx_.stopped()) {
            auto [ec, socket] = co_await acceptor_.async_accept(pool_.getIOContext());

            if (ec) {
                SPDLOG_WARN("{}", ec.message());
                continue;
            }

            if (!socket.is_open()) {
                continue;
            }

            SPDLOG_INFO("New connection from {}", socket.local_endpoint().address().to_string());

#ifdef URANUS_SSL
            const auto conn = std::make_shared<ActorConnection>(TcpSocket(std::move(socket), sslContext_), *this);
#else
            const auto conn = std::make_shared<ActorConnection>(std::move(socket), *this);
#endif

            const auto key = conn->getKey();

            bool repeated = false;

            do {
                std::unique_lock lock(mutex_);

                if (connMap_.contains(key)) {
                    repeated = true;
                    break;
                }

                connMap_.insert_or_assign(key, conn);
            } while (false);

            if (repeated) {
                SPDLOG_WARN("Connection key repeated! - {}", key);
                conn->disconnect();
                conn->attr().set("REPEATED", 1);
                continue;
            }

            // TODO: Other auth

            SPDLOG_INFO("Accept connection from: {}", conn->remoteAddress().to_string());
            conn->connect();
        }

    } catch (std::exception &e) {
        SPDLOG_ERROR(e.what());
    }
}
