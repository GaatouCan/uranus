#pragma once

#include "ServerModule.h"

#include <memory>


namespace uranus::network {
    class Package;
}
class Connection;

using std::shared_ptr;
using uranus::GameServer;
using uranus::ServerModule;
using uranus::network::Package;


class LoginAuth final : public ServerModule {

public:
    explicit LoginAuth(GameServer *ser);
    ~LoginAuth() override;

    [[nodiscard]] constexpr const char * GetModuleName() const override {
        return "Login Auth";
    }

    void OnPlayerLogin(const shared_ptr<Connection> &conn, Package *pkg);
    void OnPlayerLogout(const shared_ptr<Connection> &conn, int64_t pid);
};
