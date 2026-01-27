#include "BasePlayer.h"
#include "ActorContext.h"
#include "Envelope.h"

namespace uranus::actor {
    BasePlayer::BasePlayer() {
    }

    BasePlayer::~BasePlayer() {
    }

    void BasePlayer::sendToClient(PackageHandle &&pkg) const {
        getContext()->send(Envelope::kToClient, 0, std::move(pkg));
    }

    void BasePlayer::sendToService(const std::string &name, PackageHandle &&pkg) const {
        if (const auto sid = getContext()->queryServiceId(name); sid >= 0) {
            getContext()->send(Envelope::kToService, sid, std::move(pkg));
        }
    }
}
