#pragma once

#include <actor/Package.h>
#include <actor/ActorContext.h>

using uranus::actor::Package;
using uranus::actor::ActorContext;
using uranus::actor::ActorContextRouter;


class PlayerRouter final : public ActorContextRouter<Package> {

public:
    explicit PlayerRouter(ActorContext &ctx);
    ~PlayerRouter() override;

    void onInitial() override;
    void onTerminate() override;

    void onMessage(uint32_t src, Type *msg) override;
    void sendMessage(HandleType &&pkg) override;

    void onError(std::error_code ec) override;
    void onException(const std::exception &e) override;
};
