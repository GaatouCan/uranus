#pragma once

#include "components/ComponentModule.h"
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
        void onStart() override;
        void onTerminate() override;

        void onPackage(PackageHandle &&pkg) override;
        PackageHandle onRequest(PackageHandle &&req) override;

        void sendToClient(PackageHandle &&pkg) const;
        void sendToClient(int64_t id, const std::string &data) const;

        [[nodiscard]] int64_t getPlayerId() const;

        ComponentModule &getComponentModule();

    private:
        ComponentModule component_;
    };

}