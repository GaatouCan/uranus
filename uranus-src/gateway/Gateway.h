#pragma once

#include <actor/ServerModule.h>
#include <network/ServerBootstrap.h>

#include <unordered_map>
#include <shared_mutex>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>


namespace uranus {

    using asio::awaitable;
    using asio::co_spawn;
    using asio::detached;
    using std::shared_ptr;
    using std::unique_ptr;
    using std::make_unique;
    using std::make_shared;
    using std::shared_mutex;
    using std::unique_lock;
    using std::shared_lock;
    using std::unordered_map;

    using actor::ServerModule;
    using network::ServerBootstrap;

    class GameWorld;
    class ClientConnection;

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

        void emplace(int64_t pid, const shared_ptr<ClientConnection> &conn);
        void remove(int64_t pid);

        [[nodiscard]] shared_ptr<ClientConnection> find(int64_t pid) const;


    private:
        GameWorld &world_;

        unique_ptr<ServerBootstrap> bootstrap_;

        mutable shared_mutex mutex_;
        unordered_map<int64_t, shared_ptr<ClientConnection>> conns_;
    };
}