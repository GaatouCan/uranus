#include "Gateway.h"
#include "ClientConnection.h"
#include "GameWorld.h"
#include "player/PlayerManager.h"

#include <config/ConfigModule.h>
#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>

namespace uranus {

    using config::ConfigModule;

    Gateway::Gateway(GameWorld &world)
        : world_(world) {
    }

    Gateway::~Gateway() {

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

        bootstrap_ = std::make_unique<ServerBootstrap>();

#ifdef URANUS_SSL
        bootstrap_->useCertificateChainFile("server.crt");
        bootstrap_->usePrivateKeyFile("server.pem");
#endif

        bootstrap_->onAccept([this](TcpSocket &&socket) {
            auto conn = std::make_shared<ClientConnection>(std::move(socket));

            conn->setGateway(this);
            conn->setExpirationSecond(30);

            SPDLOG_INFO("Accept client from: {}", conn->remoteAddress().to_string());

            return conn;
        });

        bootstrap_->onRemove([](const std::string &key) {
            SPDLOG_TRACE("Remove connection: {}", key);
        });

        bootstrap_->onException([](std::exception &e) {
            SPDLOG_ERROR("Server bootstrap exception: {}", e.what());
        });

        SPDLOG_INFO("Listening on port: {}", port);
        bootstrap_->runInThread(threads, port);
    }

    void Gateway::stop() {

    }

    GameWorld &Gateway::getWorld() const {
        return world_;
    }

    void Gateway::emplace(int64_t pid, const shared_ptr<ClientConnection> &conn) {
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
            // TODO

            conn->disconnect();
            return;
        }

        // TODO
        if (auto *mgr = GET_MODULE(&world_, PlayerManager)) {
            // Create the player actor
            mgr->onPlayerLogin(pid);
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
