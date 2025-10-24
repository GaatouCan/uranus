#include "PlayerFactory.h"
#include "AbstractPlayer.h"

#include <spdlog/spdlog.h>


PlayerFactory::PlayerFactory() {
    std::string path = kPlayerLibraryDirectory;

#if defined(_WIN32) || defined(_WIN64)
    path += "/player.dll";
#else
    path += "/libplayer.so";
#endif

    library_ = SharedLibrary(path);

    if (!library_.IsValid()) {
        SPDLOG_CRITICAL("Failed to load player library");
        exit(-1);
    }

    creator_ = library_.GetSymbol<PlayerCreator>("CreatePlayer");
    if (!creator_) {
        SPDLOG_CRITICAL("Failed to load player creator");
        exit(-2);
    }

    destroyer_ = library_.GetSymbol<PlayerDestroyer>("DestroyPlayer");
    if (!destroyer_) {
        SPDLOG_CRITICAL("Failed to load player destroyer");
        exit(-3);
    }

    SPDLOG_INFO("Loaded player library");
}

PlayerFactory::~PlayerFactory() {
}

PlayerHandle PlayerFactory::CreatePlayer() {
    SPDLOG_DEBUG("Create player instance");

    auto *pPlayer = std::invoke(creator_);
    return PlayerHandle{pPlayer, this};
}

void PlayerFactory::DestroyPlayer(AbstractPlayer *pPlayer) {
    SPDLOG_DEBUG("Destroy player instance");
    std::invoke(destroyer_, pPlayer);
}
