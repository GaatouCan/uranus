#include "GamePlayer.h"

#include "../../common/ProtocolID.h"
#include "../../common/EventType.h"

#include "greeting/GreetingController.h"
#include "appearance/AppearanceController.h"

namespace gameplay {

#define HANDLE_PACKAGE(proto) \
    case k##proto: Route_##proto(this, std::move(pkg)); break;

#define HANDLE_REQUEST(proto) \
    case k##proto: return Request_##proto(this, std::move(req));

    void GamePlayer::onPackage(PackageHandle &&pkg) {
        using namespace gameplay::protocol;

        switch (pkg->id_) {
            case kPlayerQueryResult: {
                const auto data = nlohmann::json::parse(pkg->payload_.begin(), pkg->payload_.end());
                component_.deserialize(data);
            }
            break;
            HANDLE_PACKAGE(GreetingRequest)
            HANDLE_PACKAGE(AppearanceRequest)
            default: break;
        }
    }

    void GamePlayer::onEvent(const int64_t evt, DataAsset *data) {
        using event::EventType;

        switch (evt) {

            default: break;
        }
    }

    PackageHandle GamePlayer::onRequest(PackageHandle &&req) {
        using namespace gameplay::protocol;

        switch (static_cast<ProtocolID>(req->id_)) {
            HANDLE_REQUEST(PlayerInfoRequest)
            default: return nullptr;
        }
    }

#undef HANDLE_PACKAGE
#undef HANDLE_REQUEST
}
