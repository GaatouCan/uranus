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

void PlayerRouter::onMessage(uint32_t src, Type *msg) {
}

void PlayerRouter::onError(std::error_code ec) {
}

void PlayerRouter::onException(const std::exception &e) {
}

void PlayerRouter::sendMessage(HandleType &&pkg) {
}
