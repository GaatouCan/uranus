#include "PlayerRouter.h"
#include "../GameWorld.h"
#include "../gateway/Gateway.h"

using uranus::Envelope;

PlayerRouter::PlayerRouter(ActorContext &ctx)
    : ActorContextRouter(ctx),
      world_(nullptr) {
}

PlayerRouter::~PlayerRouter() {
}

void PlayerRouter::setGameWorld(GameWorld *world) {
    world_ = world;
}

GameWorld *PlayerRouter::getWorld() const {
    return world_;
}

void PlayerRouter::onInitial() {
}

void PlayerRouter::onTerminate() {
}

void PlayerRouter::onMessage(int32_t type, uint32_t src, Package *pkg) {
}

void PlayerRouter::sendMessage(const int32_t ty, uint32_t target, PackageHandle &&pkg) {
    if ((ty & Package::kToClient) != 0) {
        const auto op = getContext().attr().get<std::string>("CONNECTION_KEY");
        if (!op.has_value())
            return;

        if (const auto *gateway = getWorld()->getModule<Gateway>()) {
            if (const auto conn = gateway->find(op.value()); conn != nullptr) {
                conn->sendMessage(std::move(pkg));
            }
        }
    }

    if ((ty & Package::kToService) != 0) {
        Envelope env(ty, getContext().getId(), std::move(pkg));

        env.type |= Package::kFromPlayer;

        // TODO: Send to target service
    }
}

void PlayerRouter::onError(std::error_code ec) {
}

void PlayerRouter::onException(const std::exception &e) {
}
