#pragma once

#include <actor/ActorContext.h>

namespace uranus {

    using actor::ActorContext;

    class PlayerManager;
    class GameWorld;

    class PlayerContext final : public ActorContext {

        friend class PlayerManager;

    public:
        explicit PlayerContext(asio::io_context &ctx);
        ~PlayerContext() override;

        void send(int ty, uint32_t target, actor::PackageHandle &&pkg) override;

        [[nodiscard]] PlayerManager *getPlayerManager() const;
        [[nodiscard]] GameWorld *getWorld() const;

    private:
        void setPlayerManager(PlayerManager *mgr);

    private:
        PlayerManager *manager_;
    };
} // uranus
