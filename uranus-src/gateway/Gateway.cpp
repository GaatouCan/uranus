#include "Gateway.h"
#include "ActorConnection.h"
#include "GameWorld.h"

#include <spdlog/spdlog.h>

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
        auto *bootstrap = new network::ServerBootstrapImpl<ActorConnection>();
        // bootstrap_ = std::make_unique<network::ServerBootstrapImpl<ActorConnection>>();

#ifdef URANUS_SSL
        bootstrap->useCertificateChainFile("server.crt");
        bootstrap->usePrivateKeyFile("server.pem");
#endif

        bootstrap->onInitial([this](const std::shared_ptr<ActorConnection> &conn) {
            conn->setGateway(this);
        });

        bootstrap_ = std::unique_ptr<network::ServerBootstrapImpl<ActorConnection>>(bootstrap);

        thread_ = std::thread([this] {
            SPDLOG_INFO("Listening on port: {}", 8080);
            bootstrap_->run(4, 8080);
        });
    }

    void Gateway::stop() {

    }

    GameWorld &Gateway::getWorld() const {
        return world_;
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
