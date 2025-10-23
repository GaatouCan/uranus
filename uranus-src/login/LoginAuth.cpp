#include "LoginAuth.h"
#include "Package.h"
#include "../GameWorld.h"
#include "../gateway/Connection.h"
#include "../gateway/Gateway.h"
#include "../player/PlayerManager.h"

#include <login.pb.h>


LoginAuth::LoginAuth(GameServer *ser)
    : ServerModule(ser) {
}

LoginAuth::~LoginAuth() {
}

void LoginAuth::OnPlayerLogin(const shared_ptr<Connection> &conn, Package *pkg) {
    if (conn == nullptr || pkg == nullptr)
        return;

    if (pkg->GetPackageID() != 1001)
        return;

    Login::LoginRequest request;
    request.ParseFromString(pkg->ToString());

    conn->SetPlayerID(request.player_id());

    if (auto *gateway = GetGameServer()->GetModule<Gateway>()) {
        gateway->OnPlayerLogin(conn);
    }

    if (auto *mgr = GetGameServer()->GetModule<PlayerManager>()) {
        mgr->OnPlayerLogin(request.player_id());
    }
}

void LoginAuth::OnPlayerLogout(const shared_ptr<Connection> &conn, int64_t pid) {
}
