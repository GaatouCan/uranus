#pragma once

#include <actor/Package.h>
#include <actor/ActorContext.h>

using uranus::actor::Package;
using uranus::actor::PackageHandle;
using uranus::actor::ActorContext;
using uranus::actor::ActorContextRouter;

class GameWorld;


class PlayerRouter final : public ActorContextRouter<Package> {

public:
    explicit PlayerRouter(ActorContext &ctx);
    ~PlayerRouter() override;

    void setGameWorld(GameWorld *world);
    [[nodiscard]] GameWorld *getWorld() const;

    void onInitial() override;
    void onTerminate() override;

    void onMessage(int32_t type, uint32_t src, Package *pkg) override;
    void sendMessage(int32_t ty, uint32_t target, PackageHandle &&pkg) override;

    void onError(std::error_code ec) override;
    void onException(const std::exception &e) override;

private:
    GameWorld *world_;
};
