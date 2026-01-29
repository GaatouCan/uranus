#include "BasePlayer.h"
#include "ActorContext.h"

namespace uranus::actor {
    BasePlayer::BasePlayer() {
    }

    BasePlayer::~BasePlayer() {
    }

    void BasePlayer::sendToClient(PackageHandle &&pkg) const {
        getContext()->send(Package::kToClient, 0, std::move(pkg));
    }

    void BasePlayer::sendToService(const std::string &name, PackageHandle &&pkg) const {
        if (const auto sid = getContext()->queryActorId("service", name); sid > 0) {
            getContext()->send(Package::kToService, sid, std::move(pkg));
        }
    }
}
