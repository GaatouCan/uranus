#pragma once

#include <base/IdentAllocator.h>
#include <actor/ServerModule.h>

#include <shared_mutex>
#include <unordered_map>
#include <map>

namespace uranus {

    using actor::ServerModule;
    using std::unordered_map;
    using std::map;
    using std::shared_ptr;
    using std::shared_mutex;
    using std::unique_lock;
    using std::shared_lock;

    class GameWorld;
    class ServiceContext;

    class ServiceManager final : public ServerModule {

    public:
        explicit ServiceManager(GameWorld &world);
        ~ServiceManager() override;

        SERVER_MODULE_NAME(ServiceManager)

        void start() override;
        void stop() override;

        [[nodiscard]] GameWorld &getWorld() const;

        [[nodiscard]] shared_ptr<ServiceContext> find(uint32_t sid) const;

        [[nodiscard]] map<std::string, uint32_t> getServiceList() const;

    private:
        GameWorld &world_;

        IdentAllocator<uint32_t, true> idAlloc_;

        mutable shared_mutex mutex_;
        unordered_map<uint32_t, shared_ptr<ServiceContext>> services_;
    };
} // uranus
