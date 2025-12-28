#include "ServerBootstrap.h"
#include "BaseConnection.h"

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
    }

#ifdef URANUS_SSL
    void ServerBootstrap::useCertificateChainFile(const std::string &filename) {
        sslContext_.use_certificate_chain_file(filename);
    }

    void ServerBootstrap::usePrivateKeyFile(const std::string &filename) {
        sslContext_.use_private_key_file(filename, asio::ssl::context::pem);
    }

    void ServerBootstrap::run(const int num, const uint16_t port) {
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
            terminate();
        });

        ctx_.run();
    }

    void ServerBootstrap::terminate() {
        if (ctx_.stopped())
            return;

        guard_.reset();
        ctx_.stop();
        conns_.clear();
    }

    shared_ptr<BaseConnection> ServerBootstrap::find(const std::string &key) const {
        if (ctx_.stopped())
            return nullptr;

        shared_lock lock(mutex_);
        const auto iter = conns_.find(key);
        return iter != conns_.end() ? iter->second : nullptr;
    }

    void ServerBootstrap::remove(const std::string &key) {
        if (ctx_.stopped())
            return;

        {
            unique_lock lock(mutex_);
            conns_.erase(key);
        }

        if (onRemove_) {
            std::invoke(onRemove_, key);
        }
    }

    void ServerBootstrap::onInitial(const InitialCallback &cb) {
        onInit_ = cb;
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
                auto [ec, socket] = co_await acceptor_.async_accept(pool_.getIOContext());

                if (ec) {
                    continue;
                }

                if (!socket.is_open()) {
                    continue;
                }

#ifdef URANUS_SSL
                const auto conn = std::invoke(onInit_, *this, TcpSocket(std::move(socket), sslContext_));
#else
                const auto conn = onInit_(*this, std::move(socket));
#endif

                if (!conn)
                    continue;

                const auto key = conn->getKey();

                bool repeated = false;

                do {
                    unique_lock lock(mutex_);

                    if (conns_.contains(key)) {
                        repeated = true;
                        break;
                    }

                    conns_.insert_or_assign(key, conn);
                } while (false);

                if (repeated) {
                    conn->disconnect();
                    conn->attr().set("REPEATED", 1);
                    continue;
                }

                conn->connect();
            }
        } catch (std::exception &e) {
            if (onException_) {
                std::invoke(onException_, e);
            }
        }
    }
#endif
}
