#include "PlayerFactory.h"

#include <filesystem>
#include <spdlog/spdlog.h>

namespace uranus {
    static constexpr auto kPlayerDirectory = "player";

    PlayerFactory::PlayerFactory()
        : creator_(nullptr),
          deleter_(nullptr) {
    }

    PlayerFactory::~PlayerFactory() {
    }

    PlayerFactory &PlayerFactory::instance() {
        static PlayerFactory _instance;
        return _instance;
    }

    void PlayerFactory::initial() {
        if (const auto dir = std::filesystem::path(); !std::filesystem::exists(dir)) {
            try {
                std::filesystem::create_directories(dir);
            } catch (std::filesystem::filesystem_error &e) {
                SPDLOG_ERROR(e.what());
                exit(-1);
            }
        }

#if defined(_WIN32) || defined(_WIN64)
        const std::string filename = std::string(kPlayerDirectory) + "/player.dll";
        lib_ = SharedLibrary(std::string_view(filename));
#elif defined(__APPLE__)
        const std::string filename = std::string(kPlayerDirectory) + "/libplayer.dylib";
        lib_ = SharedLibrary(std::string_view(filename));
#elif defined(__linux__)
        const std::string filename = std::string(kPlayerDirectory) + "/libplayer.so";
        lib_ = SharedLibrary(std::string_view(filename));
#endif

        if (!lib_.available()) {
            SPDLOG_ERROR("player library not available");
            exit(-2);
        }

        creator_ = lib_.getSymbol<PlayerCreator>("CreatePlayer");
        deleter_ = lib_.getSymbol<PlayerDeleter>("DeletePlayer");

        if (!creator_ || !deleter_) {
            SPDLOG_ERROR("Failed to load player creator or deleter");
            exit(-3);
        }
    }

    BasePlayer *PlayerFactory::create() const {
        return std::invoke(creator_);
    }

    void PlayerFactory::destroy(BasePlayer *plr) {
        if (plr) {
            std::invoke(deleter_, plr);
        }
    }
}
