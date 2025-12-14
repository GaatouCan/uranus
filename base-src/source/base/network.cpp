#include "network.h"

#include <chrono>
#include <format>
#include <asio/signal_set.hpp>
#include <asio/experimental/awaitable_operators.hpp>


using namespace asio::experimental::awaitable_operators;

namespace uranus::network {
    ServerBootstrap::ServerBootstrap()
        : guard_(asio::make_work_guard(ctx_)),
          acceptor_(ctx_)
#ifdef URANUS_SSL
          , sslContext_(asio::ssl::context::tlsv13_server)
#endif
    {
    }

    ServerBootstrap::~ServerBootstrap() {
    }

    void ServerBootstrap::run(int num, uint16_t port) {
#ifdef URANUS_SSL
        sslContext_.set_options(
            asio::ssl::context::no_sslv2 |
            asio::ssl::context::no_sslv3 |
            asio::ssl::context::default_workarounds |
            asio::ssl::context::single_dh_use
        );
#endif

        pool_.start(num);

        co_spawn(ctx_, waitForClient(port), detached);

        asio::signal_set signals(ctx_, SIGINT, SIGTERM);
        signals.async_wait([this](auto, auto) {
            this->terminate();
        });

        ctx_.run();
    }

    void ServerBootstrap::terminate() {
        if (ctx_.stopped())
            return;

        guard_.reset();
        ctx_.stop();
    }

#ifdef URANUS_SSL
    void ServerBootstrap::useCertificateChainFile(const std::string &filename) {
        sslContext_.use_certificate_chain_file(filename);
    }

    void ServerBootstrap::usePrivateKeyFile(const std::string &filename) {
        sslContext_.use_private_key_file(filename, asio::ssl::context::pem);
    }
#endif

    Connection::Connection(ServerBootstrap &server, TcpSocket &&socket)
        : server_(server),
          socket_(std::move(socket)),
          watchdog_(socket_.get_executor()),
          expiration_(std::chrono::seconds(30)) {

        const auto now = std::chrono::system_clock::now();
        const auto durationSinceEpoch = now.time_since_epoch();
        const auto secondsSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(durationSinceEpoch);

#ifdef URANUS_SSL
        key_ = std::format("{}-{}", socket_.next_layer().remote_endpoint().address().to_string(), secondsSinceEpoch.count());
#else
        key_ = std::format("{}-{}", socket_.remote_endpoint().address().to_string(), secondsSinceEpoch.count());
#endif
    }

    Connection::~Connection() {
    }

    ServerBootstrap &Connection::getServerBootstrap() const {
        return server_;
    }

    TcpSocket &Connection::getSocket() {
        return socket_;
    }

    void Connection::connect() {
        received_ = std::chrono::steady_clock::now();

        co_spawn(socket_.get_executor(), [self = this->shared_from_this()]() -> awaitable<void> {
#ifdef URANUS_SSL
            if (const auto [ec] = co_await self->socket_.async_handshake(asio::ssl::stream_base::server); ec) {
                self->disconnect();
                co_return;
            }
#endif

            co_await (
                self->readMessage() &&
                self->writeMessage() &&
                self->watchdog()
            );
        }, detached);
    }

    void Connection::disconnect() {
        if (!isConnected())
            return;

        if (!attr().has("REPEATED")) {
            server_.remove(key_);
        }

#ifdef URANUS_SSL
        socket_.next_layer().close();
#else
        socket_.close();
#endif
        watchdog_.cancel();
    }

    bool Connection::isConnected() const {
#ifdef URANUS_SSL
        return socket_.next_layer().is_open();
#else
        return socket_.is_open();
#endif
    }

    const std::string &Connection::getKey() const {
        return key_;
    }

    asio::ip::address Connection::remoteAddress() const {
        return socket_.next_layer().remote_endpoint().address();
    }

    void Connection::setExpirationSecond(const int sec) {
        expiration_ = std::chrono::seconds(sec);
    }


    AttributeMap &Connection::attr() {
        return attr_;
    }


    awaitable<void> Connection::watchdog() {
        if (expiration_ <= SteadyDuration::zero())
            co_return;

        try {
            do {
                watchdog_.expires_at(received_ + expiration_);

                if (auto [ec] = co_await watchdog_.async_wait(); ec) {
                    if (ec == asio::error::operation_aborted) {
                        // TODO
                    } else {
                        this->onErrorCode(ec);
                    }

                    co_return;
                }

                this->onTimeout();

                if (isConnected()) {
                    disconnect();
                }
            } while (received_ + expiration_ > std::chrono::steady_clock::now());
        } catch (std::exception &e) {
            onException(e);
            disconnect();
        }
    }
}
