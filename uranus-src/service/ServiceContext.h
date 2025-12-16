#pragma once

#include <actor/ActorContext.h>


namespace uranus {

    using actor::ActorContext;

    class ServiceContext final : public ActorContext {

    public:
        explicit ServiceContext(asio::io_context &ctx);
        ~ServiceContext() override;

        void send(int ty, uint32_t target, actor::PackageHandle &&pkg) override;
    };
}
