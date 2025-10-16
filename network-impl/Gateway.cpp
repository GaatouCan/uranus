#include "Gateway.h"


namespace uranus::network {
    Gateway::Gateway(GameServer *server)
        : ServerModule(server) {
    }

    Gateway::~Gateway() {
    }

    void Gateway::EmplaceConnection(const int64_t pid, const shared_ptr<Connection> &conn) {
        if (conn == nullptr)
            return;

        std::shared_lock lock(mutex_);
        conn_map_[pid] = conn;
    }

    shared_ptr<Connection> Gateway::FindConnection(const int64_t pid) const {
        std::unique_lock lock(mutex_);
        const auto it = conn_map_.find(pid);
        return it == conn_map_.end() ? nullptr : it->second;
    }

    void Gateway::RemoveConnection(const int64_t pid) {
        std::unique_lock lock(mutex_);
        conn_map_.erase(pid);
    }
}
