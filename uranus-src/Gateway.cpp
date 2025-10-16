#include "Gateway.h"
#include "ConnectionInitialzer.h"
#include "GameServer.h"
#include "Connection.h"


namespace uranus::network {
    Gateway::Gateway(GameServer *server)
        : ServerModule(server),
          acceptor_(GetGameServer()->GetMainIOContext()),
          ssl_context_(asio::ssl::context::tlsv13_server) {
        ssl_context_.use_certificate_chain_file("server.crt");
        ssl_context_.use_private_key_file("server.key", asio::ssl::context::pem);
        ssl_context_.set_options(
            asio::ssl::context::no_sslv2 |
            asio::ssl::context::no_sslv3 |
            asio::ssl::context::default_workarounds |
            asio::ssl::context::single_dh_use
        );
    }

    Gateway::~Gateway() {
    }

    void Gateway::SetConnectionInitializer(ConnectionInitializer *initializer) {
        initializer_ = unique_ptr<ConnectionInitializer>(initializer);
    }

    void Gateway::Start() {
        // TODO: Load Config

        co_spawn(GetGameServer()->GetMainIOContext(), WaitForClient(8080), detached);
    }

    void Gateway::Stop() {

    }

    shared_ptr<Connection> Gateway::FindConnection(const std::string &key) const {
        std::unique_lock lock(mutex_);
        const auto it = conn_map_.find(key);
        return it == conn_map_.end() ? nullptr : it->second;
    }

    void Gateway::RemoveConnection(const std::string &key) {
        std::unique_lock lock(mutex_);
        conn_map_.erase(key);
    }

    awaitable<void> Gateway::WaitForClient(uint16_t port) {
        if (initializer_ == nullptr)
            throw std::invalid_argument("Gateway::WaitForClient - Initializer is nullptr");

        try {
            acceptor_.open(asio::ip::tcp::v4());
            acceptor_.bind({asio::ip::tcp::v4(), port});
            acceptor_.listen(port);

            while (true) {
                auto [ec, socket] = co_await acceptor_.async_accept(GetGameServer()->GetSocketIOContext());

                if (ec) {
                    // SPDLOG_ERROR("{:<20} - {}", __FUNCTION__, ec.message());
                    continue;
                }

                if (!socket.is_open())
                    continue;

                // if (auto *auth = GetServer()->GetModule<LoginAuth>()) {
                //     if (!auth->VerifyAddress(socket.remote_endpoint())) {
                //         SPDLOG_WARN("{} - Reject client from {}",
                //             __FUNCTION__, socket.remote_endpoint().address().to_string());
                //         socket.close();
                //         continue;
                //     }
                // }

                const auto conn = make_shared<Connection>(SslStream(std::move(socket), ssl_context_));

                initializer_->OnAcceptConnection(conn);

                const auto key = conn->GetKey();
                if (key.empty()) continue;

                {
                    std::unique_lock lock(mutex_);
                    if (conn_map_.contains(key)) {
                        // SPDLOG_WARN("{} - Connection[{}] has already exist", __FUNCTION__, key);
                        conn->Disconnect();
                        continue;
                    }
                    conn_map_.insert_or_assign(key, conn);
                }

                // SPDLOG_INFO("{} - New connection[{}] from {}",
                //     __FUNCTION__, key, conn->RemoteAddress().to_string());

                conn->ConnectToClient();
            }
        } catch (std::exception &e) {

        }
    }
}
