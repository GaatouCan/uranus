#include "Gateway.h"
#include "Connection.h"
#include "GameWorld.h"
#include "player/PlayerManager.h"
#include "common.h"

#include <config/ConfigModule.h>
#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>

#include <login.pb.h>

using uranus::config::ConfigModule;

namespace uranus {
    Gateway::Gateway(GameWorld &world)
        : world_(world) {
    }

    Gateway::~Gateway() {
        if (thread_.joinable()) {
            thread_.join();
        }
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

        bootstrap_->onAccept([this](ServerBootstrap &bootstrap, TcpSocket &&socket) {
            auto conn = std::make_shared<Connection>(bootstrap, std::move(socket));

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

        thread_ = std::thread([this, port, threads]() {
            SPDLOG_INFO("Listening on port: {}", port);
            bootstrap_->run(threads, port);
        });
    }

    void Gateway::stop() {

    }

    GameWorld &Gateway::getWorld() const {
        return world_;
    }

    void Gateway::onPlayerLogin(const uint32_t pid, const std::string &key) {
        if (!bootstrap_)
            return;

        if (!world_.isRunning())
            return;

        const auto conn = find(key);
        if (!conn)
            return;

        bool repeated = false;

        do {
            unique_lock lock(mutex_);

            if (pidToKey_.contains(pid)) {
                repeated = true;
                break;
            }

            pidToKey_.insert_or_assign(pid, key);
        } while (false);

        if (repeated) {
            Login::LoginRepeated res;
            res.set_player_id(pid);
            res.set_data("Player ID Repeated");

            auto pkg = Package::getHandle();

            pkg->setId(protocol::kLoginRepeated);
            pkg->setData(res.SerializeAsString());

            conn->send(std::move(pkg));

            return;
        }

        conn->attr().set("PLAYER_ID", pid);

        {
            Login::LoginSuccessResponse res;
            res.set_player_id(pid);

            auto pkg = Package::getHandle();

            pkg->setId(protocol::kServerLoginResponse);
            pkg->setData(res.SerializeAsString());

            conn->send(std::move(pkg));
        }

        if (auto *mgr = GET_MODULE(&world_, PlayerManager)) {
            // Create the player actor
            mgr->onPlayerLogin(pid);
        }
    }

    std::shared_ptr<Connection> Gateway::find(const std::string &key) const {
        if (!bootstrap_)
            return nullptr;

        return std::dynamic_pointer_cast<Connection>(bootstrap_->find(key));
    }

    std::shared_ptr<Connection> Gateway::findByPlayerID(const uint32_t pid) const {
        if (!bootstrap_)
            return nullptr;

        std::string key;
        {
            std::shared_lock lock(mutex_);
            if (const auto iter = pidToKey_.find(pid); iter != pidToKey_.end()) {
                key = iter->second;
            }
        }

        if (!key.empty()) {
            return std::dynamic_pointer_cast<Connection>(bootstrap_->find(key));
        }

        return nullptr;
    }

    void Gateway::remove(const std::string &key) const {
        if (bootstrap_) {
            bootstrap_->remove(key);
        }
    }

    void Gateway::onLogout(uint32_t pid) {
        if (!world_.isRunning())
            return;

        {
            unique_lock lock(mutex_);
            pidToKey_.erase(pid);
        }

        if (auto *mgr = GET_MODULE(&world_, PlayerManager)) {
            mgr->remove(pid);
        }
    }
}
