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

    friend class ServiceManager;

public:
    explicit ServiceContext(GameWorld *world);
    ~ServiceContext() override;

    [[nodiscard]] AbstractActor *GetActor() const override;
    [[nodiscard]] AbstractService *GetService() const;

    [[nodiscard]] GameWorld *GetWorld() const;

    [[nodiscard]] Message BuildMessage() override;

    int Initial(DataAsset *data) override;
    int Start() override;

    void Send(int64_t target, const Message &msg) override;

    void SendToService(const std::string &name, const Message &msg) override;

    void RemoteCall(int64_t target, Message req, SessionNode &&node) override;

protected:
    void CleanUp() override;

    void HandleMessage(const Message &msg) override;
    void DisposeMessage(const Message &msg) override;

private:
    void SetUpService(ServiceHandle &&handle);

private:
    ServiceHandle handle_;

    shared_ptr<PackagePool> pool_;
};