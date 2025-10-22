#pragma once

#include "ActorContext.h"
#include "../factory/ServiceHandle.h"

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
using uranus::AbstractService;
using std::shared_ptr;
using std::make_shared;

class ServiceContext final : public ActorContext {

public:
    explicit ServiceContext(GameWorld *world);
    ~ServiceContext() override;

    [[nodiscard]] AbstractActor *GetActor() const override;

    [[nodiscard]] GameWorld *GetWorld() const;

    [[nodiscard]] Message *BuildMessage() override;

    int Initial(DataAsset *data) override;
    int Start() override;

    void Send(int64_t target, Message *msg) override;

    void SendToService(const std::string &name, Message *msg) override;

    void PushMessage(Message *msg) override;

protected:
    void CleanUp() override;

private:
    void SetUpService(ServiceHandle &&handle);

private:
    ServiceHandle handle_;

    shared_ptr<PackagePool> pool_;
};