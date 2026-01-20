#include "GamePlayer.h"
#include "../../ProtocolID.h"

#include "login/LoginController.h"
#include "greeting/GreetingController.h"
#include "appearance/AppearanceController.h"

namespace gameplay {

#define HANDLE_PACKAGE(proto) \
    case ProtocolID::k##proto: Route_##proto(this, std::move(pkg)); break;

#define HANDLE_REQUEST(proto) \
    case ProtocolID::k##proto: return Request_##proto(this, std::move(req));

    void GamePlayer::onPackage(PackageHandle &&pkg) {

        using namespace gameplay::protocol;

        switch (static_cast<ProtocolID>(pkg->id_)) {
            HANDLE_PACKAGE(LoginDataResult)
            HANDLE_PACKAGE(GreetingRequest)
            HANDLE_PACKAGE(AppearanceRequest)
            default: break;
        }
    }

    PackageHandle GamePlayer::onRequest(PackageHandle &&req) {

        using namespace gameplay::protocol;

        switch (static_cast<ProtocolID>(req->id_)) {

            default: return nullptr;
        }
    }

#undef HANDLE_PACKAGE
#undef HANDLE_REQUEST
}
