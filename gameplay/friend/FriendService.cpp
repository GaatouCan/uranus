#include "FriendService.h"
#include "../common/ProtocolID.h"

#include <greeting.pb.h>

namespace gameplay {
    FriendService::FriendService() {
    }

    FriendService::~FriendService() {
    }

    std::string FriendService::getName() const {
        return "FriendService";
    }

    void FriendService::onPackage(int64_t src, PackageHandle &&pkg) {
        using namespace protocol;

        if (pkg == nullptr)
            return;

        switch (pkg->id_) {
            case kSyncPlayerInfo: {
                greeting::SyncPlayerInfo info;
                info.ParseFromArray(pkg->payload_.data(), pkg->payload_.size());
            }
            break;
            default: break;
        }

    }

    void FriendService::onEvent(int64_t src, int64_t evt, DataAsset *data) {
    }

    PackageHandle FriendService::onRequest(int64_t src, PackageHandle &&req) {
        // TODO
        return nullptr;
    }
} // gameplay

using gameplay::FriendService;

EXPORT_ACTOR_VERSION
EXPORT_CREATE_SERVICE(FriendService)
EXPORT_DELETE_SERVICE