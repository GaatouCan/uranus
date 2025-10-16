#pragma once

#include "ServerModule.h"
#include "base/Types.h"

#include <unordered_map>
#include <memory>
#include <shared_mutex>
#include <asio/ssl/context.hpp>

namespace uranus::network {

    class Connection;
    class ConnectionInitializer;

    using std::unordered_map;
    using std::shared_ptr;
    using std::unique_ptr;
    using std::make_shared;
    using std::shared_mutex;

    class Gateway final : public ServerModule {

    public:
        explicit Gateway(GameServer *server);
        ~Gateway() override;

        [[nodiscard]] constexpr const char *GetModuleName() const override { return "Gateway"; }

        void SetConnectionInitializer(ConnectionInitializer *initializer);

        void Start() override;
        void Stop() override;

        // void EmplaceConnection(int64_t pid, const shared_ptr<Connection> &conn);
        [[nodiscard]] shared_ptr<Connection> FindConnection(const std::string &key) const;

        void RemoveConnection(const std::string &key);

    private:
        awaitable<void> WaitForClient(uint16_t port);

    private:
        TcpAcceptor acceptor_;
        asio::ssl::context ssl_context_;

        mutable shared_mutex mutex_;
        unordered_map<std::string, shared_ptr<Connection>> conn_map_;

        unique_ptr<ConnectionInitializer> initializer_;
    };
}
