#pragma once

#include <actor/BaseActorContext.h>


namespace uranus {

    namespace actor {
        class BaseService;
    }

    using actor::BaseService;
    using actor::BaseActorContext;
    using actor::ActorHandle;
    using actor::PackageHandle;
    using actor::DataAssetHandle;
    using actor::ActorMap;
    using actor::ServerModule;

    class ServiceManager;
    class GameWorld;

    class ServiceContext final : public BaseActorContext {

        using super = BaseActorContext;

        friend class ServiceManager;

    public:
        ServiceContext(asio::io_context &ctx, ActorHandle &&actor);
        ~ServiceContext() override;

        [[nodiscard]] BaseService *getService() const;

        void send(int ty, int64_t target, PackageHandle &&pkg) override;

        void dispatch(int64_t evt, DataAssetHandle &&data) override;
        void listen(int64_t evt, bool cancel) override;

        [[nodiscard]] ServiceManager *getServiceManager() const;
        [[nodiscard]] GameWorld *getWorld() const;

        [[nodiscard]] ServerModule *getModule(const std::string &name) const override;

        [[nodiscard]] ActorMap getActorMap(const std::string &type) const override;
        [[nodiscard]] int64_t queryActorId(const std::string &type, const std::string &name) const override;

        void setServiceId(int64_t sid);
        [[nodiscard]] int64_t getServiceId() const;

    protected:
        void sendRequest(int ty, int64_t sess, int64_t target, PackageHandle &&pkg) override;
        void sendResponse(int ty, int64_t sess, int64_t target, PackageHandle &&pkg) override;

        bool cleanUp() override;

    private:
        void setServiceManager(ServiceManager *mgr);

    private:
        ServiceManager *manager_;
    };
}
