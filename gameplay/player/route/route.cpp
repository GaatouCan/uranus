#include "../GamePlayer.h"
#include "../../ProtocolID.h"

#include "greeting/GreetingController.h"

namespace gameplay {
    void GamePlayer::onPackage(PackageHandle &&pkg) {
        using namespace gameplay::protocol;

        switch (pkg->id_) {
            case kGreetingRequest: OnGreetingRequest(this, std::move(pkg)); break;
            default: break;
        }
    }

    PackageHandle GamePlayer::onRequest(PackageHandle &&pkg) {
        // TODO
        return nullptr;
    }
}
