#pragma once

#include <actor/ActorContext.h>


namespace uranus {

    using actor::ActorContext;

    class ServiceManager;
    class GameWorld;

    class ServiceContext final : public ActorContext {

        friend class ServiceManager;

    public:
        explicit ServiceContext(asio::io_context &ctx);
        ~ServiceContext() override;

        void send(int ty, uint32_t target, actor::PackageHandle &&pkg) override;

        [[nodiscard]] ServiceManager *getServiceManager() const;
        [[nodiscard]] GameWorld *getWorld() const;

    private:
        void setServiceManager(ServiceManager *mgr);

    private:
        ServiceManager *manager_;
    };
}
