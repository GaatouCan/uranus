#pragma once

#include <actor/BaseActorContext.h>

namespace uranus {

    using actor::BaseActorContext;
    using actor::ActorHandle;
    using actor::ServerModule;
    using actor::PackageHandle;

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
        //std::map<std::string, uint32_t> getServiceList() const override;

        void setPlayerId(int64_t pid);
        [[nodiscard]] int64_t getPlayerId() const;

    protected:
        void sendRequest(int ty, int64_t sess, int64_t target, PackageHandle &&pkg) override;
        void sendResponse(int ty, int64_t sess, int64_t target, PackageHandle &&pkg) override;

    private:
        void setPlayerManager(PlayerManager *mgr);

    private:
        PlayerManager *manager_;
    };
} // uranus
