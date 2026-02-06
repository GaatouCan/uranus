#pragma once

#include <actor/BaseService.h>

namespace gameplay {

    using uranus::actor::BaseService;
    using uranus::actor::PackageHandle;
    using uranus::actor::DataAsset;
    using uranus::actor::ActorContext;

    class FriendService final : public BaseService {

        using super = BaseService;

    public:
        FriendService();
        ~FriendService() override;

        [[nodiscard]] std::string getName() const override;

        void onInitial(ActorContext *ctx) override;

        void onPackage(int64_t src, PackageHandle &&pkg) override;
        void onEvent(int64_t evt, DataAsset *data) override;
        PackageHandle onRequest(int64_t src, PackageHandle &&req) override;
    };
} // gameplay
