#include "Gateway.h"
#include "GameWorld.h"
#include "ClientConnection.h"
#include "player/PlayerManager.h"

#include <config/ConfigModule.h>
#include <login/LoginAuth.h>

#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>


namespace uranus {

    using config::ConfigModule;

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

        shared_ptr<ClientConnection> old;

        do {
            unique_lock lock(mutex_);

            if (const auto it = conns_.find(pid); it != conns_.end()) {
                old = it->second;
                conns_.insert_or_assign(it, pid, conn);
            } else {
                conns_.insert_or_assign(pid, conn);
            }
        } while (false);

        // Send a repeated message and disconnect the old
        if (old) {
            SPDLOG_WARN("Player[{}] login repeated, redirect from[{}] to [{}]",
                pid, old->remoteAddress().to_string(), conn->remoteAddress().to_string());

            asio::dispatch(conn->socket().get_executor(), [old, pid, address = conn->remoteAddress().to_string()] {
                old->attr().erase("PLAYER_ID");
                old->attr().set("REPEATED", true);
                login::LoginAuth::sendLoginRepeated(old, pid, address);
            });
        }

        SPDLOG_INFO("Player[{}] login from: {}", pid, conn->remoteAddress().to_string());
        conn->attr().set("PLAYER_ID", pid);
        conn->attr().set("WAITING_DB", true);

        login::LoginAuth::sendLoginSuccess(conn, pid);

        if (auto *mgr = GET_MODULE(&world_, PlayerManager)) {
            mgr->onPlayerLogin(pid, conn);
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
