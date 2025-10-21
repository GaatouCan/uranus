#include "ConfigModule.h"

namespace uranus::config {
    ConfigModule::ConfigModule(GameServer *ser)
        : ServerModule(ser) {
    }

    ConfigModule::~ConfigModule() {
    }
}
