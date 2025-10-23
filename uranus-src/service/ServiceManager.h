#pragma once

#include "base/IdentAllocator.h"
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

    [[nodiscard]] int64_t FindServiceID(const std::string &name) const;

    [[nodiscard]] shared_ptr<ServiceContext> FindService(int64_t sid) const;
    [[nodiscard]] shared_ptr<ServiceContext> FindService(const std::string &name) const;

private:
    unique_ptr<ServiceFactory> factory_;

    IdentAllocator<int64_t, true> id_alloc_;

    mutable shared_mutex mutex_;
    unordered_map<int64_t, shared_ptr<ServiceContext>> services_;

    mutable shared_mutex name_mutex_;
    unordered_map<std::string, int64_t> name_to_id_;
};
