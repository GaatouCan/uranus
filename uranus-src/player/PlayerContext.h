#pragma once

#include <actor/BaseActorContext.h>

namespace uranus {

    using actor::BaseActorContext;
    using actor::ActorHandle;
    using actor::ServerModule;
    using actor::PackageHandle;
    using actor::DataAssetHandle;
    using actor::ActorMap;

    class PlayerManager;
    class GameWorld;

    class PlayerContext final : public BaseActorContext {

        friend class PlayerManager;

    public:
        PlayerContext(asio::io_context &ctx, ActorHandle &&actor);
        ~PlayerContext() override;

        void send(int ty, int64_t target, PackageHandle &&pkg) override;

        [[nodiscard]] PlayerManager *getPlayerManager() const;
        [[nodiscard]] GameWorld *getWorld() const;

        [[nodiscard]] ServerModule *getModule(const std::string &name) const override;

        [[nodiscard]] ActorMap getActorMap(const std::string &type) const override;
        [[nodiscard]] int64_t queryActorId(const std::string &type, const std::string &name) const override;

        void setPlayerId(int64_t pid);
        [[nodiscard]] int64_t getPlayerId() const;

    protected:
        void sendRequest(int ty, int64_t sess, int64_t target, PackageHandle &&pkg) override;
        void sendResponse(int ty, int64_t sess, int64_t target, PackageHandle &&pkg) override;

        void dispatch(int ty, int64_t target, int64_t evt, DataAssetHandle &&data) override;

    private:
        void setPlayerManager(PlayerManager *mgr);

    private:
        PlayerManager *manager_;
    };
} // uranus
