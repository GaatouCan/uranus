#include "PlayerFactory.h"

PlayerFactory::PlayerFactory()
    : creator_(nullptr),
      destroyer_(nullptr) {
}

PlayerFactory::~PlayerFactory() {
}

void PlayerFactory::initial() {
#if defined(_WIN32) || defined(_WIN64)
    const std::string path = "player.dll";
#elif defined(__APPLE__)
    std::string path = "libplayer.dylib"
#else
    std::string path = "libplayer.so";
#endif

    lib_ = SharedLibrary(std::string_view(path));

    if (!lib_.available()) {
        exit(-1);
    }

    creator_ = lib_.getSymbol<PlayerCreator>("CreatePlayer");
    destroyer_ = lib_.getSymbol<PlayerDestroyer>("DestroyPlayer");

    if (creator_ == nullptr || destroyer_ == nullptr) {
        exit(-2);
    }
}
