#pragma once

#include <actor/BasePlayer.h>

namespace uranus {
    using actor::BasePlayer;
    using actor::PackageHandle;
    using actor::Envelope;

    class GamePlayer final : public BasePlayer {

    public:
        void onPackage(Envelope &&envelope) override;
        PackageHandle onRequest(Envelope &&envelope) override;
    };
}
