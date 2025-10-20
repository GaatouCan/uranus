#include "LoginAuth.h"
#include "../gateway/Connection.h"

LoginAuth::LoginAuth(GameServer *ser)
    : ServerModule(ser) {
}

LoginAuth::~LoginAuth() {
}

void LoginAuth::OnPlayerLogin(const shared_ptr<Connection> &conn, Package *pkg) {
}

void LoginAuth::OnPlayerLogout(const shared_ptr<Connection> &conn, int64_t pid) {
}
