#include "Gateway.h"
#include "GameWorld.h"
#include "ClientConnection.h"
#include "player/PlayerManager.h"

#include <database/DatabaseModule.h>
#include <config/ConfigModule.h>
#include <login/LoginAuth.h>

#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>


namespace uranus {

    using config::ConfigModule;
    using database::DatabaseModule;

    Gateway::Gateway(GameWorld &world)
        : world_(world) {
        SPDLOG_DEBUG("Gateway created");
    }

    Gateway::~Gateway() {
        SPDLOG_DEBUG("Gateway destroyed");
    }

    void Gateway::start() {
        const auto *config = GET_MODULE(&world_, ConfigModule);
        if (!config) {
            SPDLOG_ERROR("Config module is not available");
            exit(-1);
        }

        const auto &cfg = config->getServerConfig();

        const auto port = cfg["server"]["network"]["port"].as<uint16_t>();
        const auto threads = cfg["server"]["network"]["threads"].as<int>();

        bootstrap_ = std::make_unique<ServerBootstrap>(threads);

#ifdef URANUS_SSL
        SPDLOG_INFO("Use certificate chain file: {}", "config/server.crt");
        SPDLOG_INFO("Use private key file: {}", "config/server.key");

        bootstrap_->useCertificateChainFile("config/server.crt");
        bootstrap_->usePrivateKeyFile("config/server.key");
#endif

        bootstrap_->onAccept([this](TcpSocket &&socket) {
            auto conn = std::make_shared<ClientConnection>(std::move(socket));

            conn->setGateway(this);
            conn->setExpirationSecond(30);

            spdlog::info("Accept client from: {}", conn->remoteAddress().to_string());

            return conn;
        });

        bootstrap_->onErrorCode([](const std::error_code ec) {
            spdlog::warn("Server bootstrap error: {}", ec.message());
        });

        bootstrap_->onException([](const std::exception &e) {
            spdlog::error("Server bootstrap exception: {}", e.what());
        });

        SPDLOG_INFO("Use IO Threads: {}", threads);
        SPDLOG_INFO("Listening on port: {}", port);

        bootstrap_->run(port);
    }

    void Gateway::stop() {
        if (bootstrap_ != nullptr) {
            bootstrap_->terminate();
        }
    }

    GameWorld &Gateway::getWorld() const {
        return world_;
    }

    void Gateway::emplace(const int64_t pid, const shared_ptr<ClientConnection> &conn) {
        if (!bootstrap_)
            return;

        if (!world_.isRunning())
            return;

        bool repeated = false;

        do {
            unique_lock lock(mutex_);

            if (conns_.contains(pid)) {
                repeated = true;
                break;
            }

            conns_.insert_or_assign(pid, conn);
        } while (false);

        if (repeated) {
            SPDLOG_WARN("Player[{}] login repeated!", pid);
            login::LoginAuth::sendLoginFailure(conn, pid, "Player ID repeated");
            return;
        }

        SPDLOG_INFO("Player[{}] login from: {}", pid, conn->remoteAddress().to_string());
        conn->attr().set("PLAYER_ID", pid);
        conn->attr().set("WAITING_DB", true);

        login::LoginAuth::sendLoginSuccess(conn, pid);

        if (auto *db = GET_MODULE(&world_, DatabaseModule)) {
            db->queryPlayer(pid, [conn, pid, world = &world_](const std::string &res) {
                login::LoginAuth::sendLoginPlayerResult(conn, pid, "Query from database success");

                asio::post(conn->socket().get_executor(), [conn] {
                    conn->attr().set("WAITING_DB", false);
                });

                // Run in main thread
                asio::post(world->getIOContext(), [&] {
                    if (auto *mgr = GET_MODULE(world, PlayerManager)) {
                        mgr->onPlayerLogin(pid, res);
                    }
                });
            });
        }
    }

    void Gateway::remove(const int64_t pid) {
        if (!world_.isRunning())
            return;

        {
            unique_lock lock(mutex_);
            conns_.erase(pid);
        }

        if (auto *mgr = GET_MODULE(&world_, PlayerManager)) {
            mgr->onPlayerLogout(pid);
        }
    }

    shared_ptr<ClientConnection> Gateway::find(const int64_t pid) const {
        if (bootstrap_ == nullptr)
            return nullptr;

        if (!world_.isRunning())
            return nullptr;

        shared_lock lock(mutex_);
        const auto it = conns_.find(pid);
        return it != conns_.end() ? it->second : nullptr;
    }
}
