#include "Gateway.h"
#include "ClientConnection.h"
#include "GameWorld.h"
#include "player/PlayerManager.h"
#include "common.h"

// #include <config/ConfigModule.h>
// #include <yaml-cpp/yaml.h>
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
        // const auto *config = GET_MODULE(&world_, ConfigModule);
        // if (!config) {
        //     SPDLOG_ERROR("Config module is not available");
        //     exit(-1);
        // }
        //
        // const auto &cfg = config->getServerConfig();
        //
        // const auto port = cfg["server"]["network"]["port"].as<uint16_t>();
        // const auto threads = cfg["server"]["network"]["threads"].as<int>();

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

        thread_ = std::thread([this]() {
            SPDLOG_INFO("Listening on port: {}", 8090);
            bootstrap_->run(4, 8090);
        });
    }

    void Gateway::stop() {

    }

    GameWorld &Gateway::getWorld() const {
        return world_;
    }

    void Gateway::emplace(int64_t pid, const shared_ptr<ClientConnection> &conn) {
    }

    void Gateway::remove(int64_t pid) {
    }

    shared_ptr<ClientConnection> Gateway::find(int64_t pid) const {
    }
}
