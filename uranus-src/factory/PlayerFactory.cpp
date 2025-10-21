#include "PlayerFactory.h"
#include "AbstractPlayer.h"

#include <spdlog/spdlog.h>


PlayerFactory::PlayerFactory()
    : creator_(nullptr),
      destroyer_(nullptr) {
}

PlayerFactory::~PlayerFactory() {
}

void PlayerFactory::Initial() {
    std::string agent = kPlayerAgentDirectory;

#if defined(_WIN32) || defined(_WIN64)
    agent += "/agent.dll";
#else
    agent += "/libagent.so";
#endif


    library_ = SharedLibrary(agent);

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

PlayerHandle PlayerFactory::CreatePlayer() {
    SPDLOG_DEBUG("Create player instance");

    auto *pPlayer = std::invoke(creator_);
    return PlayerHandle{pPlayer, this};
}

void PlayerFactory::DestroyPlayer(AbstractPlayer *pPlayer) {
    SPDLOG_DEBUG("Destroy player instance");
    std::invoke(destroyer_, pPlayer);
}
