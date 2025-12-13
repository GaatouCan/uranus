#include "ActorConnection.h"

ActorConnection::ActorConnection(TcpSocket &&socket)
    : ConnectionImpl(std::move(socket)){
}

ActorConnection::~ActorConnection() {
}

void ActorConnection::onReadMessage(PackageHandle &&pkg) {

}

void ActorConnection::beforeWrite(Package *pkg) {
}

void ActorConnection::afterWrite(PackageHandle &&pkg) {
}

void ActorConnection::onTimeout() {
}
