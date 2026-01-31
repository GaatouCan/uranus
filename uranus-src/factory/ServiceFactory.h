#pragma once

#include <base/SharedLibrary.h>
#include <base/Singleton.h>
#include <tuple>
#include <unordered_map>
#include <atomic>

namespace uranus {

    namespace actor {
        class BaseService;
    }

    using actor::BaseService;
    using std::unordered_map;
    using std::tuple;
    using std::make_tuple;
    using std::atomic_uint32_t;

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


    private:
        struct ServiceNode {

            SharedLibrary lib;
            ServiceCreator ctor = nullptr;
            ServiceDeleter del = nullptr;
            atomic_uint32_t count = 0;

            ServiceNode();
            ~ServiceNode();

            ServiceNode(const ServiceNode &rhs);
            ServiceNode &operator=(const ServiceNode &rhs);

            ServiceNode(ServiceNode &&rhs) noexcept;
            ServiceNode &operator=(ServiceNode &&rhs) noexcept;
        };

        unordered_map<std::string, ServiceNode> coreServices_;
        unordered_map<std::string, ServiceNode> extendServices_;
    };
} // uranus
