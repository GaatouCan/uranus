#pragma once

#include <actor/BasePlayer.h>

namespace gameplay {

    using uranus::actor::BasePlayer;
    using uranus::actor::PackageHandle;
    using uranus::actor::ActorContext;

    class GamePlayer final : public BasePlayer {

        using super = BasePlayer;

    public:
        GamePlayer();
        ~GamePlayer() override;

        void onInitial(ActorContext *ctx) override;

        void onTerminate() override;

        void onPackage(PackageHandle &&pkg) override;
        PackageHandle onRequest(PackageHandle &&pkg) override;

        void sendToClient(PackageHandle &&pkg) const;
    };

}