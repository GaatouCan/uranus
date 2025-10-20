#include "PlayerContext.h"

PlayerContext::PlayerContext(GameServer *ser)
    : ActorContext(ser) {
}

PlayerContext::~PlayerContext() {
}

AbstractActor *PlayerContext::GetActor() const {
}

int PlayerContext::Initial(DataAsset *data) {
}

int PlayerContext::Start() {
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
