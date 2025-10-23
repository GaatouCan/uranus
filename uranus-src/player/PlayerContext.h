#pragma once

#include "ActorContext.h"
#include "../factory/PlayerHandle.h"

#include <memory>

namespace uranus::network {
    class PackagePool;
}
class GameWorld;

using uranus::network::PackagePool;
using uranus::DataAsset;
using uranus::ActorContext;
using uranus::GameServer;
using uranus::Message;
using uranus::AbstractActor;
using uranus::AbstractPlayer;
using std::shared_ptr;
using std::make_shared;


class PlayerContext final : public ActorContext {

    friend class PlayerManager;

public:
    explicit PlayerContext(GameWorld *world);
    ~PlayerContext() override;

    [[nodiscard]] AbstractActor *GetActor() const override;
    [[nodiscard]] AbstractPlayer *GetPlayer() const;

    [[nodiscard]] GameWorld *GetWorld() const;

    int Initial(DataAsset *data) override;
    int Start() override;

    [[nodiscard]] Message BuildMessage() override;

    void Send(int64_t target, const Message &msg) override;

    void SendToService(const std::string &name, const Message &msg) override;

    void RemoteCall(int64_t target, Message req, SessionNode &&node) override;

protected:
    void CleanUp() override;

    void HandleMessage(const Message &msg) override;
    void DisposeMessage(const Message &msg) override;

private:
    void SetUpPlayer(PlayerHandle &&handle);

private:
    PlayerHandle handle_;

    shared_ptr<PackagePool> pool_;
};
