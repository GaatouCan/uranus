#pragma once

#include <base/Singleton.h>
#include <base/SharedLibrary.h>
#include <tuple>
#include <unordered_map>

namespace uranus {

    namespace actor {
        class BaseService;
    }

    using actor::BaseService;
    using std::unordered_map;
    using std::tuple;
    using std::make_tuple;

    using ServiceCreator = BaseService *(*)();
    using ServiceDeleter = void (*)(BaseService *);

    class ServiceFactory final : public Singleton<ServiceFactory> {

        friend class Singleton;

        ServiceFactory();
        ~ServiceFactory() override;

    public:
        using InstanceResult = tuple<BaseService *, std::filesystem::path>;

        void initial();

        [[nodiscard]] InstanceResult create(const std::string &path) const;
        void destroy(BaseService *ptr, const std::string &path);


    private:
        struct ServiceNode {
            SharedLibrary lib;
            ServiceCreator ctor = nullptr;
            ServiceDeleter del = nullptr;
        };

        unordered_map<std::string, ServiceNode> coreServices_;
        unordered_map<std::string, ServiceNode> extendServices_;
    };
} // uranus
