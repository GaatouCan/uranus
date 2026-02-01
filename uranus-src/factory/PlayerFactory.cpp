#include "PlayerFactory.h"

#include <actor/BasePlayer.h>
#include <filesystem>
#include <spdlog/spdlog.h>


namespace uranus {
    static constexpr auto kPlayerDirectory = "player";

    static bool VerifyLibraryVersion(const SharedLibrary &lib) {
        using actor::ActorVersion;
        using actor::kUranusActorABIVersion;
        using actor::kUranusActorAPIVersion;
        using actor::kUranusActorHeaderVersion;
        using VersionGetter = const ActorVersion* (*)();

        auto *getter = lib.getSymbol<VersionGetter>("GetActorVersion");
        if (getter == nullptr) {
            SPDLOG_ERROR("Failed to get player library version");
            return false;
        }

        const auto *ver = getter();
        if (ver == nullptr) {
            SPDLOG_ERROR("Failed to get player library version");
            return false;
        }

        if (ver->abi_version != kUranusActorABIVersion ||
            ver->api_version != kUranusActorAPIVersion ||
            ver->header_version != kUranusActorHeaderVersion
        ) {
            SPDLOG_ERROR("Player library version is not match!!!");
            return false;
        }

        return true;
    }

    PlayerFactory::PlayerFactory()
        : creator_(nullptr),
          deleter_(nullptr),
          count_(0) {
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
            SPDLOG_ERROR("Player library not available");
            exit(-2);
        }

        if (!VerifyLibraryVersion(lib_)) {
            exit(-3);
        }

        creator_ = lib_.getSymbol<PlayerCreator>("CreatePlayer");
        deleter_ = lib_.getSymbol<PlayerDeleter>("DeletePlayer");

        if (!creator_ || !deleter_) {
            SPDLOG_ERROR("Failed to load player creator or deleter");
            exit(-3);
        }

        SPDLOG_INFO("Use player library: {}", filename);
    }

    PlayerFactory::InstanceResult PlayerFactory::create() {
        shared_lock lock(mutex_);

        if (creator_ == nullptr) {
            SPDLOG_ERROR("Failed to create player creator");
            return std::make_tuple(nullptr, "");
        }

        auto *inst = std::invoke(creator_);
        count_.fetch_add(1, std::memory_order_acq_rel);

        return std::make_tuple(inst, lib_.path());
    }

    void PlayerFactory::destroy(BasePlayer *plr) {
        if (plr) {
            shared_lock lock(mutex_);

            if (deleter_ == nullptr) {
                SPDLOG_ERROR("Failed to destroy player creator");
                delete plr;
            }

            std::invoke(deleter_, plr);
            count_.fetch_sub(1, std::memory_order_acq_rel);
        }
    }

    void PlayerFactory::release() {
        if (count_.load(std::memory_order_acquire) > 0) {
            SPDLOG_WARN("Player library is still in use");
            return;
        }

        SharedLibrary temp;

        {
            unique_lock lock(mutex_);
            creator_ = nullptr;
            deleter_ = nullptr;
            lib_.swap(temp);
        }

        if (temp.tryRelease()) {
            SPDLOG_INFO("Release player library success");
        }
    }

    void PlayerFactory::reload() {
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
            SPDLOG_ERROR("Player library not available");
            return;
        }

        if (!VerifyLibraryVersion(lib_)) {
            return;
        }

        creator_ = lib_.getSymbol<PlayerCreator>("CreatePlayer");
        deleter_ = lib_.getSymbol<PlayerDeleter>("DeletePlayer");

        if (!creator_ || !deleter_) {
            creator_ = nullptr;
            deleter_ = nullptr;
            lib_.reset();
            SPDLOG_ERROR("Failed to load player creator or deleter");
            return;
        }

        SPDLOG_INFO("Reload player library success");
    }
}
