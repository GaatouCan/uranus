#pragma once

#include "ServerModule.h"

#include <unordered_map>
#include <memory>
#include <shared_mutex>

namespace uranus::network {

    class Connection;

    using std::unordered_map;
    using std::shared_ptr;
    using std::shared_mutex;

    class Gateway final : public ServerModule {

    public:
        explicit Gateway(GameServer *server);
        ~Gateway() override;

        [[nodiscard]] constexpr const char *GetModuleName() const override { return "Gateway"; }

        void EmplaceConnection(int64_t pid, const shared_ptr<Connection> &conn);
        [[nodiscard]] shared_ptr<Connection> FindConnection(int64_t pid) const;

        void RemoveConnection(int64_t pid);

    private:
        mutable shared_mutex mutex_;
        unordered_map<int64_t, shared_ptr<Connection>> conn_map_;
    };
}