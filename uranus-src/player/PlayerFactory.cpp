#include "PlayerFactory.h"

PlayerFactory::PlayerFactory() {
}

PlayerFactory::~PlayerFactory() {
}

PlayerFactory &PlayerFactory::instance() {
    static PlayerFactory inst;
    return inst;
}
