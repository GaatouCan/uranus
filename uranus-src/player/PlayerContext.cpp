#include "PlayerContext.h"

PlayerContext::PlayerContext(GameServer *ser)
    : ActorContext(ser) {
}

PlayerContext::~PlayerContext() {
}

AbstractActor *PlayerContext::GetActor() const {
    return nullptr;
}

int PlayerContext::Initial(DataAsset *data) {
    return 0;
}

int PlayerContext::Start() {
    return 0;
}

void PlayerContext::Stop() {
}

void PlayerContext::SendToService(int64_t target, Message *msg) {
}

void PlayerContext::SendToService(const std::string &name, Message *msg) {
}

void PlayerContext::SendToPlayer(int64_t pid, Message *msg) {
}

void PlayerContext::SendToClient(int64_t pid, Message *msg) {
}

void PlayerContext::PushMessage(Message *msg) {
}
