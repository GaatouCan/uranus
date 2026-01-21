#include <spdlog/spdlog.h>

#include "GameWorld.h"
#include "gateway/Gateway.h"
#include "gateway/ClientConnection.h"
#include "player/PlayerManager.h"
#include "service/ServiceManager.h"

#include <config/ConfigModule.h>
#include <logger/LoggerModule.h>
#include <login/LoginAuth.h>
#include <database/DatabaseModule.h>


using uranus::network::Connection;
using uranus::GameWorld;
using uranus::config::ConfigModule;
using uranus::login::LoginAuth;
using uranus::logger::LoggerModule;
using uranus::database::DatabaseModule;
using uranus::PlayerManager;
using uranus::ServiceManager;
using uranus::Gateway;
using uranus::ClientConnection;


int main() {
    spdlog::info("Hello World!");

    auto *world = new GameWorld();

    world->pushModule(new ConfigModule());
    world->pushModule(new LoggerModule());
    world->pushModule(new LoginAuth());
    world->pushModule(new DatabaseModule());
    world->pushModule(new PlayerManager(*world));
    world->pushModule(new ServiceManager(*world));
    world->pushModule(new Gateway(*world));

    // Setup login auth
    {
        auto *login = GET_MODULE(world, LoginAuth);

        login->onLoginSuccess([world](const std::shared_ptr<Connection> &conn, const int64_t pid) {
            const auto client = std::dynamic_pointer_cast<ClientConnection>(conn);
            if (!client)
                return;

            if (auto *gateway = GET_MODULE(world, Gateway)) {
                gateway->emplace(pid, client);
            }
        });

        login->onLoginFailure(
            [](const std::shared_ptr<Connection> &conn, const int64_t pid, const std::string &reason) {
                LoginAuth::sendLoginFailure(conn, pid, reason);
            });

        login->onPlayerLogout(
            [](const std::shared_ptr<Connection> &conn, const int64_t pid, const std::string &reason) {
                LoginAuth::sendLogoutResponse(conn, reason);
            });
    }

    world->run();
    world->terminate();

    delete world;

    return 0;
}
