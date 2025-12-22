#include "Gateway.h"
#include "ActorConnection.h"
#include "GameWorld.h"

#include <config/ConfigModule.h>
#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>

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
        const auto *config = dynamic_cast<ConfigModule *>(world_.getModule("ConfigModule"));
        if (!config) {
            SPDLOG_ERROR("Config module is not available");
            exit(-1);
        }

        const auto &cfg = config->getServerConfig();

        const auto port = cfg["server"]["network"]["port"].as<uint16_t>();
        const auto threads = cfg["server"]["network"]["threads"].as<int>();

        auto *bootstrap = new network::ServerBootstrapImpl<ActorConnection>();

#ifdef URANUS_SSL
        bootstrap->useCertificateChainFile("server.crt");
        bootstrap->usePrivateKeyFile("server.pem");
#endif

        bootstrap->onInitial([this](const std::shared_ptr<ActorConnection> &conn) {
            conn->setGateway(this);

            SPDLOG_INFO("Accept client from {}", conn->remoteAddress().to_string());
            return true;
        });

        bootstrap_ = std::unique_ptr<network::ServerBootstrapImpl<ActorConnection>>(bootstrap);

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

        unique_lock lock(mutex_);
        pidToKey_[pid] = key;
    }

    std::shared_ptr<ActorConnection> Gateway::find(const std::string &key) const {
        if (!bootstrap_)
            return nullptr;

        return std::dynamic_pointer_cast<ActorConnection>(bootstrap_->find(key));
    }

    std::shared_ptr<ActorConnection> Gateway::findByPlayerID(const uint32_t pid) const {
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
            return std::dynamic_pointer_cast<ActorConnection>(bootstrap_->find(key));
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

        unique_lock lock(mutex_);
        pidToKey_.erase(pid);
    }
}
