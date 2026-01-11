#pragma once

#include <base/Singleton.h>
#include <base/SharedLibrary.h>

#include <unordered_map>

namespace uranus {

    namespace actor {
        class BaseService;
    }
    using actor::BaseService;

    using ServiceCreator = BaseService *(*)();
    using ServiceDeleter = void (*)(BaseService *);

    class ServiceFactory final : public Singleton<ServiceFactory> {

        friend class Singleton;

        ServiceFactory();
        ~ServiceFactory() override;

    public:
        void initial();

        BaseService *create(const std::string &path);
        void destroy(BaseService *ptr, const std::string &path);

        // template<typename Fn>
        // void foreachServiceLibrary(Fn &&fn) {
        //     for (const auto &[key, val] : coreServices_) {
        //         std::invoke(fn, key, val, true);
        //     }
        //
        //     for (const auto &[key, val] : extendServices_) {
        //         std::invoke(fn, key, val, false);
        //     }
        // }

    private:
        struct ServiceNode {
            SharedLibrary lib;
            ServiceCreator ctor = nullptr;
            ServiceDeleter del = nullptr;
        };

        std::unordered_map<std::string, ServiceNode> coreServices_;
        std::unordered_map<std::string, ServiceNode> extendServices_;
    };
} // uranus
