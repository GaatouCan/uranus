#include <spdlog/spdlog.h>

#include <network/Connection.h>
#include "Gateway/ActorConnection.h"

using uranus::network::ServerBootstrapImpl;

int main() {
    spdlog::info("Hello World!");

    auto *sb = new ServerBootstrapImpl<ActorConnection>();

    sb->onInitial([](const std::shared_ptr<ActorConnection> &conn) {

    });

    sb->run(4, 8080);
    sb->terminate();

    delete sb;

    return 0;
}
