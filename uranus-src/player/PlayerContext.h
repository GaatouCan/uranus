#pragma once

#include <actor/ActorContext.h>

namespace uranus {

    using actor::ActorContext;
    using actor::ServerModule;
    using actor::PackageHandle;

    class PlayerManager;
    class GameWorld;

    class PlayerContext final : public ActorContext {

        friend class PlayerManager;

    public:
        explicit PlayerContext(asio::io_context &ctx);
        ~PlayerContext() override;

        void send(int ty, uint32_t target, PackageHandle &&pkg) override;

        [[nodiscard]] PlayerManager *getPlayerManager() const;
        [[nodiscard]] GameWorld *getWorld() const;

        [[nodiscard]] ServerModule *getModule(const std::string &name) const override;
        std::map<std::string, uint32_t> getServiceList() const override;

    protected:
        void sendRequest(int ty, uint32_t sess, uint32_t target, PackageHandle &&pkg) override;
        void sendResponse(int ty, uint32_t sess, uint32_t target, PackageHandle &&pkg) override;

    private:
        void setPlayerManager(PlayerManager *mgr);

    private:
        PlayerManager *manager_;
    };
} // uranus
