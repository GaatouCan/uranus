#include "ActorConnection.h"

ActorConnection::ActorConnection(ServerBootstrap &server, TcpSocket &&socket)
    : ConnectionImpl<PackageCodec>(server, std::move(socket)){
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
