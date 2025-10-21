#pragma once

#include "ServerModule.h"

#include <memory>
#include <shared_mutex>
#include <unordered_map>


class ServiceFactory;
class ServiceContext;

using std::shared_ptr;
using std::unique_ptr;
using std::make_shared;
using std::make_unique;
using std::unordered_map;
using std::shared_mutex;
using std::shared_lock;
using std::unique_lock;
using uranus::ServerModule;
using uranus::GameServer;

class ServiceManager final : public ServerModule {

public:
    explicit ServiceManager(GameServer *ser);
    ~ServiceManager() override;

    [[nodiscard]] constexpr const char *GetModuleName() const override {
        return "Service Manager";
    }

    void Start() override;
    void Stop() override;

    [[nodiscard]] shared_ptr<ServiceContext> FindService(int64_t sid) const;

private:
    unique_ptr<ServiceFactory> factory_;

    mutable shared_mutex mutex_;
    unordered_map<int64_t, shared_ptr<ServiceContext>> services_;
};
