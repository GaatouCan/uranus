#include "ClientConnection.h"
#include "Gateway.h"
#include "GameWorld.h"

#include "player/PlayerManager.h"
#include "player/PlayerContext.h"

#include <login/LoginAuth.h>
#include <login/LoginProtocol.h>
#include <spdlog/spdlog.h>

namespace uranus {

    using actor::Package;
    using actor::Envelope;
    using login::LoginAuth;

    ClientConnection::ClientConnection(TcpSocket &&socket)
        : ConnectionAdapter(std::move(socket)),
          gateway_(nullptr) {
    }

    ClientConnection::~ClientConnection() = default;

    Gateway *ClientConnection::getGateway() const {
        return gateway_;
    }

    GameWorld *ClientConnection::getWorld() const {
        if (gateway_)
            return &gateway_->getWorld();
        return nullptr;
    }

    void ClientConnection::onConnect() {
    }

    void ClientConnection::onDisconnect() {
        SPDLOG_INFO("Client[{}] disconnected", attr().get<std::string>("CONNECTION_KEY").value());

        const auto op = attr().get<int64_t>("PLAYER_ID");
        if (!op.has_value()) {
            return;
        }

        const auto pid = op.value();
        gateway_->remove(pid);
    }

    void ClientConnection::onReadMessage(PackageHandle &&pkg) {
        if (!pkg)
            return;

        const auto op = attr().get<int64_t>("PLAYER_ID");

        // Not login
        if (!op.has_value()) {
            if (auto *auth = GET_MODULE(getWorld(), LoginAuth)) {
                auth->onLoginRequest(std::move(pkg), shared_from_this());
            }
            return;
        }

        const auto pid = op.value();

        const auto *mgr = GET_MODULE(getWorld(), PlayerManager);
        if (!mgr)
            return;

        const auto ctx = mgr->find(pid);
        if (!ctx)
            return;

        Envelope evl;

        evl.type = (Envelope::kFromClient | Envelope::kToPlayer);
        evl.source = pid;
        evl.variant = std::move(pkg);

        ctx->pushEnvelope(std::move(evl));
    }

    void ClientConnection::beforeWrite(Package *pkg) {
    }

    void ClientConnection::afterWrite(PackageHandle &&pkg) {
        if (pkg == nullptr)
            return;

        // Fail to login or request to logout
        if (pkg->getId() == login::kLoginFailure) {
            SPDLOG_WARN("Client from [{}] login failed", remoteAddress().to_string());
            disconnect();
            return;
        }

        if (pkg->getId() == login::kLogoutResponse) {
            SPDLOG_INFO("Client from [{}] request to logout", remoteAddress().to_string());
            disconnect();
            return;
        }
    }

    void ClientConnection::onTimeout() {
    }

    void ClientConnection::onErrorCode(std::error_code ec) {
        SPDLOG_ERROR(ec.message());
    }

    void ClientConnection::onException(std::exception &e) {
        SPDLOG_ERROR(e.what());
    }

    void ClientConnection::setGateway(Gateway *gateway) {
        gateway_ = gateway;
    }
}
