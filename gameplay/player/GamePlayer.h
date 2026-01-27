#pragma once

#include "components/ComponentModule.h"

#include <actor/BasePlayer.h>
#include <google/protobuf/message_lite.h>

namespace gameplay {

    using uranus::actor::BasePlayer;
    using uranus::actor::PackageHandle;
    using uranus::actor::DataAsset;
    using uranus::actor::ActorContext;
    using google::protobuf::MessageLite;

    inline constexpr int kPlayerQueryResult = 1051;

    class GamePlayer final : public BasePlayer {

        using super = BasePlayer;

    public:
        GamePlayer();
        ~GamePlayer() override;

        void onInitial(ActorContext *ctx) override;
        void onStart(DataAsset *data) override;
        void onTerminate() override;

        void save();

        void onLogin();
        void onLogout();

        void onPackage(PackageHandle &&pkg) override;
        void onEvent(int64_t evt, DataAsset *data) override;
        PackageHandle onRequest(PackageHandle &&req) override;

        template<class T>
        requires std::derived_from<T, MessageLite>
        void sendToClient(int64_t id, const T &msg) const;

        [[nodiscard]] int64_t getPlayerId() const;

        ComponentModule &getComponentModule();

    private:
        ComponentModule component_;
    };

    template<class T>
    requires std::derived_from<T, MessageLite>
    void GamePlayer::sendToClient(const int64_t id, const T &msg) const {
        auto pkg = uranus::actor::Package::getHandle();

        pkg->setId(id);

        pkg->payload_.resize(msg.ByteSizeLong());
        msg.SerializeToArray(pkg->payload_.data(), pkg->payload_.size());

        this->sendToClient(std::move(pkg));
    }
}
