#pragma once

#include <actor/ActorContext.h>


namespace uranus {

    namespace actor {
        class BaseService;
    }

    using actor::BaseService;
    using actor::ActorContext;
    using actor::PackageHandle;

    class ServiceManager;
    class GameWorld;

    class ServiceContext final : public ActorContext {

        friend class ServiceManager;

    public:
        explicit ServiceContext(asio::io_context &ctx);
        ~ServiceContext() override;

        [[nodiscard]] BaseService *getService() const;

        void send(int ty, uint32_t target, PackageHandle &&pkg) override;

        [[nodiscard]] ServiceManager *getServiceManager() const;
        [[nodiscard]] GameWorld *getWorld() const;

    private:
        void setServiceManager(ServiceManager *mgr);

    private:
        ServiceManager *manager_;
    };
}
