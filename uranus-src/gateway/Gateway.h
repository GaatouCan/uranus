#pragma once

#include <actor/ServerModule.h>
#include <base/network.h>

#include <unordered_map>
#include <shared_mutex>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>


namespace uranus {

    using asio::awaitable;
    using asio::co_spawn;
    using asio::detached;

    using actor::ServerModule;
    using network::ServerBootstrap;

    class GameWorld;
    class ActorConnection;

    class Gateway final : public ServerModule {
    public:
        Gateway() = delete;

        explicit Gateway(GameWorld &world);
        ~Gateway() override;

        DISABLE_COPY_MOVE(Gateway)

        SERVER_MODULE_NAME(Gateway)

        void start() override;
        void stop() override;

        [[nodiscard]] GameWorld &getWorld() const;

        [[nodiscard]] std::shared_ptr<ActorConnection> find(const std::string &key) const;
        [[nodiscard]] std::shared_ptr<ActorConnection> findByPlayerID(uint32_t pid) const;

        void remove(const std::string &key) const;

        void onLogout(uint32_t pid);

    private:
        GameWorld &world_;

        std::thread thread_;
        std::unique_ptr<ServerBootstrap> bootstrap_;

        mutable std::shared_mutex mutex_;
        std::unordered_map<uint32_t, std::string> pidToKey_;
    };
}