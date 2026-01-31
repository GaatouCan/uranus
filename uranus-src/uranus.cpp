#include <mimalloc.h>
#include <mimalloc-new-delete.h>
#include <spdlog/spdlog.h>

#include "GameWorld.h"
#include "gateway/Gateway.h"
#include "gateway/ClientConnection.h"
#include "player/PlayerManager.h"
#include "service/ServiceManager.h"
#include "event/EventManager.h"

#include <config/ConfigModule.h>
#include <logger/LoggerModule.h>
#include <login/LoginAuth.h>
#include <database/DatabaseModule.h>

#include <asio.hpp>

using uranus::network::Connection;
using uranus::GameWorld;
using uranus::config::ConfigModule;
using uranus::login::LoginAuth;
using uranus::logger::LoggerModule;
using uranus::database::DatabaseModule;
using uranus::EventManager;
using uranus::PlayerManager;
using uranus::ServiceManager;
using uranus::Gateway;
using uranus::ClientConnection;


int main() {
    spdlog::info("Using mi-malloc version: {}", mi_version());

    auto *world = new GameWorld();

    world->pushModule<ConfigModule>();
    world->pushModule<LoggerModule>();
    // world->pushModule<LoginAuth>();
    world->pushModule<DatabaseModule>();
    world->pushModule<EventManager>(*world);
    world->pushModule<PlayerManager>(*world);
    world->pushModule<ServiceManager>(*world);
    world->pushModule<Gateway>(*world);

    // Setup login auth
    // {
    //     auto *login = GET_MODULE(world, LoginAuth);
    //
    //     login->onLoginSuccess([world](const std::shared_ptr<Connection> &conn, const int64_t pid) {
    //         const auto client = std::dynamic_pointer_cast<ClientConnection>(conn);
    //         if (!client)
    //             return;
    //
    //         if (auto *gateway = GET_MODULE(world, Gateway)) {
    //             gateway->emplace(pid, client);
    //         }
    //     });
    //
    //     login->onLoginFailure(
    //         [](const std::shared_ptr<Connection> &conn, const int64_t pid, const std::string &reason) {
    //             LoginAuth::sendLoginFailure(conn, pid, reason);
    //         });
    //
    //     login->onPlayerLogout(
    //         [](const std::shared_ptr<Connection> &conn, const int64_t pid, const std::string &reason) {
    //             LoginAuth::sendLogoutResponse(conn, reason);
    //         });
    // }

    // Fot test
    {
        using asio::co_spawn;
        using asio::awaitable;
        using asio::detached;
        co_spawn(world->getIOContext(), [&]() -> awaitable<void> {
            auto exec = co_await asio::this_coro::executor;
            uranus::SteadyTimer timer(exec, std::chrono::seconds(2));
            co_await timer.async_wait();

            world->terminate();
        }, detached);
    }

    world->run();

    delete world;
    return 0;
}
