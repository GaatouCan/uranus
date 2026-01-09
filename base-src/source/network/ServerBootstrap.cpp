#include "ServerBootstrap.h"
#include "AbstractConnection.h"

#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/signal_set.hpp>

namespace uranus::network {

    using asio::detached;

    ServerBootstrap::ServerBootstrap()
        : guard_(asio::make_work_guard(ctx_)),
          acceptor_(ctx_)
#ifdef URANUS_SSL
          , sslContext_(asio::ssl::context::tlsv13_server)
#endif
    {
    }

    ServerBootstrap::~ServerBootstrap() {
        for (auto &val: pool_) {
            if (val.joinable()) {
                val.join();
            }
        }
    }

#ifdef URANUS_SSL
    void ServerBootstrap::useCertificateChainFile(const std::string &filename) {
        sslContext_.use_certificate_chain_file(filename);
    }

    void ServerBootstrap::usePrivateKeyFile(const std::string &filename) {
        sslContext_.use_private_key_file(filename, asio::ssl::context::pem);
    }
#endif

    void ServerBootstrap::run(const int num, const uint16_t port) {
#ifdef URANUS_SSL
        sslContext_.set_options(
            asio::ssl::context::no_sslv2 |
            asio::ssl::context::no_sslv3 |
            asio::ssl::context::default_workarounds |
            asio::ssl::context::single_dh_use
        );
#endif

        for (auto i = 0; i < num; i++) {
            pool_.emplace_back([this] {
                ctx_.run();
            });
        }

        co_spawn(ctx_, waitForClient(port), detached);

        asio::signal_set signals(ctx_, SIGINT, SIGTERM);
        signals.async_wait([this](auto, auto) {
            terminate();
        });

        ctx_.run();
    }

    void ServerBootstrap::terminate() {
        if (ctx_.stopped())
            return;

        guard_.reset();
        ctx_.stop();
    }

    void ServerBootstrap::onAccept(const AcceptCallback &cb) {
        onAccept_ = cb;
    }

    void ServerBootstrap::onRemove(const RemoveCallback &cb) {
        onRemove_ = cb;
    }

    void ServerBootstrap::onException(const ExceptionCallback &cb) {
        onException_ = cb;
    }

    awaitable<void> ServerBootstrap::waitForClient(const uint16_t port) {
        try {
            acceptor_.open(asio::ip::tcp::v4());
            acceptor_.bind({asio::ip::tcp::v4(), port});
            acceptor_.listen(port);

            while (!ctx_.stopped()) {
                auto [ec, socket] = co_await acceptor_.async_accept();

                if (ec) {
                    continue;
                }

                if (!socket.is_open()) {
                    continue;
                }

#ifdef URANUS_SSL
                const auto conn = std::invoke(onAccept_, TcpSocket(std::move(socket), sslContext_));
#else
                const auto conn = std::invoke(onAccept_, std::move(socket));
#endif

                if (!conn)
                    continue;

                conn->connect();
            }
        } catch (std::exception &e) {
            if (onException_) {
                std::invoke(onException_, e);
            }
        }
    }
}
