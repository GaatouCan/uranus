#pragma once

#include <actor/BaseActorContext.h>

namespace uranus {

    using actor::BaseActorContext;
    using actor::ActorHandle;
    using actor::ServerModule;
    using actor::PackageHandle;
    using actor::DataAssetHandle;
    using actor::ActorMap;
    using actor::CommandHandler;

    class PlayerManager;
    class GameWorld;

    class PlayerContext final : public BaseActorContext {

        using super = BaseActorContext;

        friend class PlayerManager;

    public:
        PlayerContext(asio::any_io_executor ctx, ActorHandle &&actor);
        ~PlayerContext() override;

        [[nodiscard]] PlayerManager *getPlayerManager() const;
        [[nodiscard]] GameWorld *getWorld() const;

        [[nodiscard]] ServerModule *getModule(const std::string &name) const override;

        void send(int ty, int64_t target, PackageHandle &&pkg) override;

        void dispatch(int64_t evt, DataAssetHandle &&data) override;
        void listen(int64_t evt, bool cancel) override;

        [[nodiscard]] ActorMap getActorMap(const std::string &type) const override;
        [[nodiscard]] int64_t queryActorId(const std::string &type, const std::string &name) const override;

        void setPlayerId(int64_t pid);
        [[nodiscard]] int64_t getPlayerId() const;

    protected:
        void sendRequest(int ty, int64_t sess, int64_t target, PackageHandle &&pkg) override;
        void sendResponse(int ty, int64_t sess, int64_t target, PackageHandle &&pkg) override;

        void createCommand(const std::string &cmd, DataAssetHandle &&data, CommandHandler &&handler) override;

        bool cleanUp() override;

    private:
        void setPlayerManager(PlayerManager *mgr);

    private:
        PlayerManager *manager_;
    };
} // uranus
