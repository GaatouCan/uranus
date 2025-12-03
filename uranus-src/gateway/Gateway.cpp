#include "Gateway.h"
#include "../GameWorld.h"

Gateway::Gateway(ServerBootstrap &server)
    : ServerModule(server) {
}

Gateway::~Gateway() {
}

GameWorld &Gateway::getWorld() const {
    return dynamic_cast<GameWorld &>(getServer());
}

void Gateway::start() {
}

void Gateway::stop() {
}
