#include "PlayerRouter.h"

PlayerRouter::PlayerRouter(ActorContext &ctx)
    : ActorContextRouter(ctx) {
}

PlayerRouter::~PlayerRouter() {
}

void PlayerRouter::onInitial() {
}

void PlayerRouter::onTerminate() {
}

void PlayerRouter::onMessage(int32_t type, uint32_t src, Package *pkg) {

}

void PlayerRouter::sendMessage(int32_t ty, uint32_t target, PackageHandle &&pkg) {

}

void PlayerRouter::onError(std::error_code ec) {
}

void PlayerRouter::onException(const std::exception &e) {
}
