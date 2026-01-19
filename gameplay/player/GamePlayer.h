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

        [[nodiscard]] int64_t getPlayerId() const;

        ComponentModule &getComponentModule();

    private:
        ComponentModule component_;
    };

#define SEND_TO_CLIENT(plr, id, msg)                                    \
{                                                                       \
    auto pkg = uranus::actor::Package::getHandle();                     \
    pkg->setId(static_cast<int64_t>(protocol::ProtocolID::id));         \
                                                                        \
    pkg->payload_.resize((msg).ByteSizeLong());                         \
    (msg).SerializeToArray(pkg->payload_.data(), pkg->payload_.size()); \
                                                                        \
    (plr).sendToClient(std::move(pkg));                                 \
}

}