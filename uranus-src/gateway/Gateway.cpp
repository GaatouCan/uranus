#include "Gateway.h"
#include "Connection.h"
#include "ConfigModule.h"
#include "Package.h"
#include "../player/PlayerManager.h"
#include "../GameWorld.h"
#include "../other/FixedPackageID.h"

#include <spdlog/spdlog.h>
#include <login.pb.h>


using uranus::config::ConfigModule;

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

void Gateway::Start() {
    const auto &config = GetGameServer()->GetModule<ConfigModule>()->GetServerConfig();

    const auto port = config["server"]["port"].as<uint16_t>();

    co_spawn(GetGameServer()->GetMainIOContext(), WaitForClient(port), detached);
}

void Gateway::Stop() {
}

shared_ptr<Connection> Gateway::FindConnection(const std::string &key) const {
    std::shared_lock lock(mutex_);
    const auto it = conn_map_.find(key);
    return it == conn_map_.end() ? nullptr : it->second;
}

shared_ptr<Connection> Gateway::FindConnection(const int64_t pid) const {
    std::string key;

    {
        std::shared_lock lock(player_mutex_);
        const auto iter = pid_to_key_.find(pid);
        if (iter == pid_to_key_.end())
            return nullptr;

        key = iter->second;
    }

    if (key.empty())
        return nullptr;

    return FindConnection(key);
}

void Gateway::OnPlayerLogin(const shared_ptr<Connection> &conn) {
    const auto pid = conn->GetPlayerID();

    if (const shared_ptr<Connection> old = FindConnection(pid); old != nullptr) {
        const auto old_key = old->GetKey();

        SPDLOG_WARN("Player[{}] - Has already logged in, remove and disconnect the old connection[{}]",
            pid, old_key);

        // As soon as possible remove the old connection
        RemoveConnection(old_key, pid);

        auto msg = old->BuildMessage();
        msg.type = (Message::kFromServer | Message::kToClient);

        Login::LoginRepeated res;
        res.set_player_id(pid);
        res.set_addr(conn->RemoteAddress().to_string());

        auto *pkg = static_cast<Package *>(msg.data);

        pkg->SetPackageID(static_cast<int32_t>(FixedPackageID::kLoginRepeated));
        pkg->SetData(res.SerializeAsString());

        old->SendToClient(msg);
    }

    auto *mgr = GetGameServer()->GetModule<PlayerManager>();
    if (const auto ret = mgr->OnPlayerLogin(pid); ret != 1) {
        SPDLOG_WARN("Player[{}] - Login failed with code {}", pid, ret);

        // As soon as possible remove the connection
        RemoveConnection(conn->GetKey(), pid);

        auto msg = conn->BuildMessage();
        msg.type = (Message::kFromServer | Message::kToClient);

        Login::LoginFailed res;
        res.set_player_id(pid);
        res.set_result(ret);

        auto *pkg = static_cast<Package *>(msg.data);

        pkg->SetPackageID(static_cast<int32_t>(FixedPackageID::kLoginFailed));
        pkg->SetData(res.SerializeAsString());

        conn->SendToClient(msg);

        return;
    }

    // Add mapping to player id
    {
        std::unique_lock lock(player_mutex_);
        pid_to_key_[conn->GetPlayerID()] = conn->GetKey();
    }

    SPDLOG_INFO("Player[{}] - Login success with connection[{}]", pid, conn->GetKey());

    auto msg = conn->BuildMessage();
    msg.type = (Message::kFromServer | Message::kToClient);

    Login::LoginSuccess res;
    res.set_player_id(pid);

    auto *pkg = static_cast<Package *>(msg.data);

    pkg->SetPackageID(static_cast<int32_t>(FixedPackageID::kLoginSuccess));
    pkg->SetData(res.SerializeAsString());

    conn->SendToClient(msg);
}

void Gateway::RemoveConnection(const std::string &key, const int64_t pid) {
    if (!GetGameServer()->IsRunning())
        return;

    {
        std::unique_lock lock(mutex_);
        conn_map_.erase(key);
    }

    if (pid > 0) {
        std::unique_lock lock(player_mutex_);
        pid_to_key_.erase(pid);
    }

    SPDLOG_INFO("Remove connection[{}] for player[{}]", key, pid);
}

awaitable<void> Gateway::WaitForClient(uint16_t port) {
    try {
        acceptor_.open(asio::ip::tcp::v4());
        acceptor_.bind({asio::ip::tcp::v4(), port});
        acceptor_.listen(port);

        SPDLOG_INFO("Waiting for client on port: {}", port);

        while (true) {
            auto [ec, socket] = co_await acceptor_.async_accept(GetGameServer()->GetSocketIOContext());

            if (ec) {
                SPDLOG_ERROR("{}", ec.message());
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

            const auto conn = make_shared<Connection>(SslStream(std::move(socket), ssl_context_), this);

            const auto key = conn->GetKey();
            if (key.empty()) continue;

            {
                std::unique_lock lock(mutex_);
                if (conn_map_.contains(key)) {
                    SPDLOG_WARN("Connection[{}] has already exist", key);
                    conn->Disconnect();
                    continue;
                }
                conn_map_.insert_or_assign(key, conn);
            }

            SPDLOG_INFO("New connection[{}] from {}", key, conn->RemoteAddress().to_string());
            conn->ConnectToClient();
        }
    } catch (std::exception &e) {
        SPDLOG_ERROR("{}", e.what());
    }
}
