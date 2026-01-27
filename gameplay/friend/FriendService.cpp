//
// Created by admin on 2026/1/27.
//

#include "FriendService.h"

namespace gameplay {
    FriendService::FriendService() {
    }

    FriendService::~FriendService() {
    }

    std::string FriendService::getName() const {
        return "FriendService";
    }

    void FriendService::onPackage(PackageHandle &&pkg) {
    }

    void FriendService::onEvent(int64_t evt, DataAsset *data) {
    }

    PackageHandle FriendService::onRequest(PackageHandle &&req) {
        // TODO
        return nullptr;
    }
} // gameplay

using gameplay::FriendService;

ACTOR_EXPORT FriendService *CreateInstance() {
    return new FriendService();
}

ACTOR_EXPORT void DeleteInstance(FriendService *ser) {
    delete ser;
}