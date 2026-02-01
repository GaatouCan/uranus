#pragma once

#include <base/SharedLibrary.h>
#include <base/Singleton.h>
#include <unordered_map>
#include <shared_mutex>
#include <atomic>
#include <tuple>
#include <memory>

namespace uranus {

    namespace actor {
        class BaseService;
    }

    using actor::BaseService;
    using std::unordered_map;
    using std::tuple;
    using std::make_tuple;
    using std::unique_ptr;
    using std::make_unique;
    using std::atomic_uint32_t;
    using std::shared_mutex;
    using std::unique_lock;
    using std::shared_lock;

    using ServiceCreator = BaseService *(*)();
    using ServiceDeleter = void (*)(BaseService *);

    class ServiceFactory final : public Singleton<ServiceFactory> {

        friend class Singleton;

        ServiceFactory();
        ~ServiceFactory() override;

    public:
        using InstanceResult = tuple<BaseService *, std::filesystem::path>;

        void initial();

        [[nodiscard]] InstanceResult create(const std::string &path);
        void destroy(BaseService *ptr, const std::string &path);

        void release(const std::string &path);
        void reload(const std::string &path);

    private:
        struct ServiceNode {
            SharedLibrary   library;
            ServiceCreator  creator;
            ServiceDeleter  deleter;
            atomic_uint32_t count;
        };

        mutable shared_mutex mutex_;
        unordered_map<std::string, unique_ptr<ServiceNode>> coreMap_;
        unordered_map<std::string, unique_ptr<ServiceNode>> extendMap_;
    };
} // uranus
