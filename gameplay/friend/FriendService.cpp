#include "FriendService.h"
#include "common/ProtocolID.h"

#include <actor/ActorContext.h>
#include <logger/LoggerModule.h>

#include <greeting.pb.h>


namespace gameplay {

    using uranus::logger::LoggerModule;

    FriendService::FriendService() {
        enableTick_ = true;
    }

    FriendService::~FriendService() {
    }

    std::string FriendService::getName() const {
        return "FriendService";
    }

    void FriendService::onInitial(ActorContext *ctx) {
        super::onInitial(ctx);

        if (auto *module = ACTOR_GET_MODULE(LoggerModule)) {
            module->createLogger("friend_service", "friend");
        }
    }

    void FriendService::onPackage(int64_t src, PackageHandle &&pkg) {
        using namespace protocol;

        if (pkg == nullptr)
            return;

        auto logger = spdlog::get("friend_service");

        switch (pkg->id_) {
            case kSyncPlayerInfo: {
                greeting::SyncPlayerInfo info;
                info.ParseFromArray(pkg->payload_.data(), pkg->payload_.size());

                logger->info("Sync player[{}] info", src);
            }
            break;
            default: break;
        }

    }

    void FriendService::onEvent(int64_t evt, DataAsset *data) {
    }

    PackageHandle FriendService::onRequest(int64_t src, PackageHandle &&req) {
        // TODO
        return nullptr;
    }
} // gameplay

using gameplay::FriendService;
EXPORT_SERVICE(FriendService)