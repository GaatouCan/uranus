#include "GameWorldHandler.h"

namespace uranus {
    GameWorldHandler::GameWorldHandler(Connection &conn)
        : ConnectionHandler(conn){
    }

    GameWorldHandler::~GameWorldHandler() {
    }

    void GameWorldHandler::onReceive(HandleType &&msg) {
    }

    void GameWorldHandler::onWrite(Type *msg) {
    }
} // uranus