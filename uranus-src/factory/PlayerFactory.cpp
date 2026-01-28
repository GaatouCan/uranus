#include "PlayerFactory.h"

#include <actor/BaseActor.h>
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

    void PlayerFactory::initial() {
        if (const auto dir = std::filesystem::path(kPlayerDirectory); !std::filesystem::exists(dir)) {
            try {
                std::filesystem::create_directories(dir);
            } catch (std::filesystem::filesystem_error &e) {
                SPDLOG_ERROR(e.what());
                exit(-1);
            }
        }

#if defined(_WIN32) || defined(_WIN64)
        const std::string filename = std::string(kPlayerDirectory) + "/player.dll";
        lib_ = SharedLibrary(filename);
#elif defined(__APPLE__)
        const std::string filename = std::string(kPlayerDirectory) + "/libplayer.dylib";
        lib_ = SharedLibrary(filename);
#elif defined(__linux__)
        const std::string filename = std::string(kPlayerDirectory) + "/libplayer.so";
        lib_ = SharedLibrary(filename);
#endif

        if (!lib_.available()) {
            SPDLOG_ERROR("player library not available");
            exit(-2);
        }

        {
            using actor::ActorVersion;
            using actor::kUranusActorABIVersion;
            using actor::kUranusActorAPIVersion;
            using actor::kUranusActorHeaderVersion;
            using VersionGetter = const ActorVersion* (*)();

            auto *getter = lib_.getSymbol<VersionGetter>("GetActorVersion");
            if (getter == nullptr) {
                SPDLOG_ERROR("Failed to get player library version");
                exit(-2);
            }

            const auto *ver = getter();
            if (ver == nullptr) {
                SPDLOG_ERROR("Failed to get player library version");
                exit(-2);
            }

            if (ver->abi_version != kUranusActorABIVersion ||
                ver->api_version != kUranusActorAPIVersion ||
                ver->header_version != kUranusActorHeaderVersion
            ) {
                SPDLOG_ERROR("Player library version is not match!!!");
                exit(-2);
            }
        }

        creator_ = lib_.getSymbol<PlayerCreator>("CreatePlayer");
        deleter_ = lib_.getSymbol<PlayerDeleter>("DeletePlayer");

        if (!creator_ || !deleter_) {
            SPDLOG_ERROR("Failed to load player creator or deleter");
            exit(-3);
        }

        SPDLOG_INFO("Use player library: {}", filename);
    }

    PlayerFactory::InstanceResult PlayerFactory::create() const {
        auto *inst = std::invoke(creator_);
        return std::make_tuple(inst, lib_.path());
    }

    void PlayerFactory::destroy(BasePlayer *plr) {
        if (plr) {
            std::invoke(deleter_, plr);
        }
    }
}
