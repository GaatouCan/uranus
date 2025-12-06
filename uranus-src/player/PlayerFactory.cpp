#include "PlayerFactory.h"

PlayerFactory::PlayerFactory()
    : creator_(nullptr),
      destroyer_(nullptr) {
}

PlayerFactory::~PlayerFactory() = default;

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

ActorHandle PlayerFactory::create() {
    if (creator_ == nullptr)
        return nullptr;

    auto *plr = std::invoke(creator_);

    return ActorHandle{plr, [this](uranus::actor::BaseActor *ptr) {
        if (auto *player = dynamic_cast<BasePlayer *>(ptr)) {
            destroy(player);
            return;
        }
        delete ptr;
    }};
}

void PlayerFactory::destroy(BasePlayer *plr) {
    if (plr == nullptr)
        return;

    std::invoke(destroyer_, plr);
}
