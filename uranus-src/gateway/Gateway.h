#pragma once

#include <actor/ServerModule.h>
#include <base/types.h>
#include <base/MultiIOContextPool.h>

#include <unordered_map>
#include <shared_mutex>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>


using asio::awaitable;
using asio::co_spawn;
using asio::detached;
using uranus::actor::ServerModule;
using uranus::TcpAcceptor;
using uranus::MultiIOContextPool;

class GameWorld;
class ActorConnection;

class Gateway final : public ServerModule {
public:
    Gateway() = delete;

    explicit Gateway(GameWorld &world);
    ~Gateway() override;

    DISABLE_COPY_MOVE(Gateway)

    [[nodiscard]] constexpr const char *getModuleName() override {
        return "Gateway";
    }

    void start() override;
    void stop() override;

    [[nodiscard]] std::shared_ptr<ActorConnection> find(const std::string &key) const;
    [[nodiscard]] std::shared_ptr<ActorConnection> findByPlayerID(uint32_t pid) const;

    void remove(const std::string &key);

private:
    awaitable<void> waitForClient(uint16_t port);

private:
    GameWorld &world_;

    asio::io_context ctx_;
    asio::executor_work_guard<asio::io_context::executor_type> guard_;

    TcpAcceptor acceptor_;

#ifdef URANUS_SSL
    asio::ssl::context sslContext_;
#endif

    std::thread thread_;
    MultiIOContextPool pool_;

    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<ActorConnection>> connMap_;
    std::unordered_map<uint32_t, std::string> pidToKey_;
};
