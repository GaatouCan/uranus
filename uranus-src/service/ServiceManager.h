#pragma once

#include <base/IdentAllocator.h>
#include <actor/ServerModule.h>

#include <shared_mutex>
#include <unordered_map>
#include <string>
#include <map>
#include <set>


namespace uranus {

    using actor::ServerModule;
    using std::unordered_map;
    using std::map;
    using std::set;
    using std::shared_ptr;
    using std::shared_mutex;
    using std::unique_lock;
    using std::shared_lock;

    using ServiceMap = map<std::string, int64_t>;
    using ServiceHashMap = unordered_map<std::string, int64_t>;

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

        [[nodiscard]] shared_ptr<ServiceContext> find(int64_t sid) const;
        [[nodiscard]] set<shared_ptr<ServiceContext>> getServiceSet(const set<int64_t> &sids) const;

        [[nodiscard]] ServiceMap getServiceMap() const;
        [[nodiscard]] int64_t queryServiceId(const std::string &name) const;

    private:
        void updateServiceCache();

    private:
        GameWorld &world_;

        IdentAllocator<int64_t, true> idAlloc_;

        mutable shared_mutex mutex_;
        unordered_map<int64_t, shared_ptr<ServiceContext>> services_;

        ServiceHashMap nameToId_;
        mutable shared_mutex cacheMutex_;
    };
} // uranus
