#include "LoginAuth.h"
#include "Package.h"
#include "../GameWorld.h"
#include "../gateway/Connection.h"
#include "../gateway/Gateway.h"
#include "../other/FixedPackageID.h"

#include <login.pb.h>


LoginAuth::LoginAuth(GameServer *ser)
    : ServerModule(ser) {
}

LoginAuth::~LoginAuth() {
}

void LoginAuth::OnPlayerLogin(const shared_ptr<Connection> &conn, const Package *pkg) const {
    if (conn == nullptr || pkg == nullptr)
        return;

    if (pkg->GetPackageID() != static_cast<int>(FixedPackageID::kLoginRequest))
        return;

    Login::LoginRequest request;
    request.ParseFromString(pkg->ToString());

    conn->SetPlayerID(request.player_id());

    auto *gateway = GetGameServer()->GetModule<Gateway>();
    gateway->OnPlayerLogin(conn);
}

void LoginAuth::OnPlayerLogout(const shared_ptr<Connection> &conn, int64_t pid) {
}
